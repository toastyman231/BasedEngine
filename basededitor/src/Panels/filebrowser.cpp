#include "based/pch.h"
#include "filebrowser.h"

#include "based/input/keyboard.h"
#include "external/imgui/imgui.h"
#include "external/imgui/imgui_internal.h"
#include "external/tfd/tinyfiledialogs.h"

namespace editor::panels
{
	void FileBrowser::Initialize()
	{
		mImporters.emplace_back(new MeshImporter());
		mImporters.emplace_back(new ModelImporter());
		mImporters.emplace_back(new TextureImporter());
		mImporters.emplace_back(new MaterialImporter());
		mImporters.emplace_back(new AnimationImporter());

		std::queue<std::string> dirs;
		dirs.push(Statics::GetProjectDirectory());

		while (!dirs.empty())
		{
			auto path = std::filesystem::canonical(dirs.front());
			auto dir = std::filesystem::directory_entry(path);
			BASED_TRACE("Visiting {}", path.string());
			dirs.pop();

			for (auto file : std::filesystem::directory_iterator(path))
			{
				if (file.is_directory())
				{
					if (std::find(mExcludeDirs.begin(), mExcludeDirs.end(), 
						file.path().filename().string()) == mExcludeDirs.end())
						dirs.push(file.path().string());
				}
				else
				{
					InstantiateFile(file.path().string());
				}
			}
		}

	}

	void FileBrowser::Render()
	{
		ImGui::Begin(mPanelTitle.c_str());
		const ImGuiID dockspaceID = ImGui::GetID("FileBrowserDockspace");

		if (!ImGui::DockBuilderGetNode(dockspaceID))
		{
			ImGui::DockBuilderAddNode(dockspaceID, ImGuiDockNodeFlags_DockSpace | ImGuiDockNodeFlags_NoWindowMenuButton |
				ImGuiDockNodeFlags_NoCloseButton);
			ImGui::DockBuilderSetNodeSize(dockspaceID, ImVec2(350.f, 200.f));

			ImGuiID leftDockID = 0, rightDockID = 0;
			ImGui::DockBuilderSplitNode(dockspaceID, ImGuiDir_Left,
				0.5f, &leftDockID, &rightDockID);

			ImGuiDockNode* pLeftNode = ImGui::DockBuilderGetNode(leftDockID);
			ImGuiDockNode* pRightNode = ImGui::DockBuilderGetNode(rightDockID);
			pLeftNode->LocalFlags |= ImGuiDockNodeFlags_NoTabBar | ImGuiDockNodeFlags_NoDockingSplitMe
				| ImGuiDockNodeFlags_NoDockingOverMe;
			pRightNode->LocalFlags |= ImGuiDockNodeFlags_NoTabBar | ImGuiDockNodeFlags_NoDockingSplitMe
				| ImGuiDockNodeFlags_NoDockingOverMe;

			ImGui::DockBuilderDockWindow("File Hierarchy", leftDockID);
			ImGui::DockBuilderDockWindow("File Viewer", rightDockID);

			ImGui::DockBuilderFinish(dockspaceID);
		}

		ImGui::DockSpace(dockspaceID, ImVec2(0.f, 0.f), 0);

		ImGui::End();

		if (ImGui::Begin("File Hierarchy"))
		{
			mCurrentDirIndex = 0;
			mCurrentDirCount = 0;
			DrawDirectoryTree(Statics::GetProjectDirectory());
		}
		ImGui::End();

		if (ImGui::Begin("File Viewer"))
		{
			ImVec2 pos = ImGui::GetCursorScreenPos();

			auto textHeight = ImGui::GetTextLineHeightWithSpacing();
			float numRows = based::math::Ceil((float)(mCurrentFileIndex + 1) / (float)mNumColumns);
			float browserFullHeight = numRows * mFileSize.y + textHeight * numRows + 10.f;
			browserFullHeight = std::max(browserFullHeight, ImGui::GetContentRegionAvail().y);

			if (ImGui::InvisibleButton("##bg", ImVec2(ImGui::GetContentRegionMax().x, browserFullHeight)))
			{
				Statics::SetSelectedFiles({});
				mFileMultiSelectBegin = -1;
				mFileMultiSelectEnd = -1;
				mRenamePath = "";
			}
				
			ImGui::SetItemAllowOverlap();

			ImGui::SetCursorScreenPos(pos);
			DrawFileViewer();
			mIsFileViewerHovered = ImGui::IsWindowHovered();

			if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(1))
				ImGui::OpenPopup("FilesContext");
			DrawContextMenu();
		}
		ImGui::End();
	}

	bool FileBrowser::HandleFileDrop(const std::string& path)
	{
		if (Statics::GetSelectedDirectories().empty() || Statics::GetSelectedDirectories().size() != 1) return false;

		auto fromPath = std::filesystem::path(path);
		auto fileName = fromPath.filename();
		auto toPath = std::filesystem::canonical(Statics::GetSelectedDirectories()[0]);

		if (std::filesystem::copy_file(fromPath, toPath / fileName))
		{
			auto result = InstantiateFile(path);

			if (!result) BASED_ERROR("Error importing file {}", path);

			return result;
		}

		return false;
	}

	bool FileBrowser::IsDirectorySelected(const std::filesystem::path& path)
	{
		return Statics::SelectedDirectoriesContains(path);
	}

	bool FileBrowser::IsFileSelected(const std::filesystem::path& path)
	{
		return Statics::SelectedFilesContains(path);
	}

	bool FileBrowser::IsDirectoryLeaf(const std::filesystem::path& path)
	{
		for (auto dir : std::filesystem::directory_iterator(path))
		{
			if (dir.is_directory()) return false;
		}

		return true;
	}

	bool FileBrowser::IsFileOfType(const std::filesystem::path& path, const std::string& type)
	{
		return path.string().find(type) != std::string::npos;
	}

	bool FileBrowser::IsFileViewerHovered()
	{
		return mIsFileViewerHovered;
	}

	bool FileBrowser::DoesProjectContainFile(const std::string& filename, std::filesystem::path& outPath)
	{
		std::queue<std::filesystem::path> dirs;
		dirs.emplace(std::filesystem::canonical(Statics::GetProjectDirectory()));
		while (!dirs.empty())
		{
			auto dir = dirs.front();
			dirs.pop();

			if (dir.filename().string().find(filename) != std::string::npos)
			{
				outPath = dir;
				return true;
			}

			for (const auto& child : std::filesystem::directory_iterator(dir))
			{
				if (child.is_directory() && std::find(mExcludeDirs.begin(), mExcludeDirs.end(), 
					child.path().filename().string()) == mExcludeDirs.end())
				{
					dirs.emplace(child.path());
					continue;
				}

				if (child.path().filename().string().find(filename) != std::string::npos)
				{
					outPath = child.path();
					return true;
				}
			}
		}

		return false;
	}

	void FileBrowser::DrawDirectoryTree(const std::string& path)
	{
		for (auto dir :
			std::filesystem::directory_iterator(path))
		{
			auto dirName = dir.path().filename().string();
			if (std::find(mExcludeDirs.begin(), mExcludeDirs.end(), dirName) == mExcludeDirs.end())
			{
				ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_OpenOnArrow;
				if (dir.is_directory() && IsDirectoryLeaf(dir.path())) 
					flags |= ImGuiTreeNodeFlags_Leaf;
				if (IsDirectorySelected(dir.path())) 
					flags |= ImGuiTreeNodeFlags_Selected;

				if (!dir.is_directory()) continue;

				++mCurrentDirIndex;

				if (!Statics::GetSelectedDirectories().empty())
				{
					if (Statics::GetSelectedDirectories().back() == dir.path())
						mDirMultiSelectBegin = mCurrentDirIndex;
				}

				int32_t min = std::min(mDirMultiSelectBegin, mDirMultiSelectEnd);
				int32_t max = std::max(mDirMultiSelectBegin, mDirMultiSelectEnd);
				mDirCountMax = max - min;

				if (mDirMultiSelectEnd != -1 && mCurrentDirIndex >= min
					&& mCurrentDirIndex <= max)
				{
					if (!IsDirectorySelected(dir.path()))
						Statics::AddSelectedDirectory(dir.path());
					mCurrentDirCount++;

					if (mCurrentDirCount >= mDirCountMax + 1)
					{
						mDirMultiSelectBegin = -1;
						mDirMultiSelectEnd = -1;
					}
				}

				const bool isOpen = ImGui::TreeNodeEx(dirName.c_str(), flags);

				if (ImGui::IsItemHovered() && (ImGui::IsMouseClicked(0) || ImGui::IsMouseClicked(1)) &&
					!(based::input::Keyboard::Key(BASED_INPUT_KEY_LSHIFT)
						|| based::input::Keyboard::Key(BASED_INPUT_KEY_LCTRL)))
				{
					Statics::SetSelectedDirectories({ dir.path() });
				}
				else if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(0)
					&& based::input::Keyboard::Key(BASED_INPUT_KEY_LCTRL))
				{

					if (!IsDirectorySelected(dir.path()))
					{
						Statics::AddSelectedDirectory(dir.path());
					}
					else
					{
						Statics::RemoveSelectedDirectory(dir.path());
					}
				}
				else if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0)
					&& based::input::Keyboard::Key(BASED_INPUT_KEY_LSHIFT))
				{
					if (mDirMultiSelectBegin == -1)
					{
						Statics::SetSelectedDirectories({ dir.path() });
						mDirMultiSelectBegin = mCurrentDirIndex;
					} else
					{
						mDirMultiSelectEnd = mCurrentDirIndex;
					}
				}

				if (dir.is_directory())
					if (isOpen) DrawDirectoryTree(dir.path().string());

				if (isOpen) ImGui::TreePop();
			}
		}
	}

	void FileBrowser::DrawFileViewer()
	{
		ImVec2 itemSize = mFileSize;
		mNumColumns = (int)std::floor(ImGui::GetContentRegionAvail().x / itemSize.x);
		ImGui::Columns(mNumColumns, 0, false);

		mCurrentFileIndex = 0;
		mCurrentFileCount = 0;
		for (auto& p : Statics::GetSelectedDirectories())
		{
			auto path = std::filesystem::path(p);
			std::deque<std::filesystem::directory_entry> directories;
			
			for (auto dir : std::filesystem::directory_iterator(path))
			{
				if (dir.is_directory())
					directories.push_front(dir);
				else
					directories.push_back(dir);
			}

			for (auto dir : directories)
			{
				++mCurrentFileIndex;
				ImVec2 pos = ImGui::GetCursorScreenPos();

				if (!Statics::GetSelectedFiles().empty())
				{
					if (Statics::GetSelectedFiles().back() == dir.path().string())
						mFileMultiSelectBegin = mCurrentFileIndex;
				}

				int32_t min = std::min(mFileMultiSelectBegin, mFileMultiSelectEnd);
				int32_t max = std::max(mFileMultiSelectBegin, mFileMultiSelectEnd);
				mFileCountMax = max - min;

				if (mFileMultiSelectEnd != -1 && mCurrentFileIndex >= min
					&& mCurrentFileIndex <= max)
				{
					if (!IsFileSelected(dir.path().string()))
						Statics::AddSelectedFile(dir.path().string());
					mCurrentFileCount++;

					if (mCurrentFileCount >= mFileCountMax + 1)
					{
						mFileMultiSelectBegin = -1;
						mFileMultiSelectEnd = -1;
					}
				}

				bool selected = IsFileSelected(dir.path().string());
				ImGui::Selectable(
					"##fileicon",
					&selected, ImGuiSelectableFlags_AllowDoubleClick,
					itemSize);
				ImGui::SetItemAllowOverlap();

				ImGui::SetCursorScreenPos({ pos.x - 4, pos.y });
				ImGui::Image(
					(void*)static_cast<intptr_t>(GetIconByFileType(dir.path().filename().string())),
					itemSize, { 0, 1 }, { 1, 0 });
				if (mScrollTarget == dir.path().string())
				{
					ImGui::ScrollToBringRectIntoView(ImGui::GetCurrentWindow(), 
						ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax()));
					mScrollTarget = "";
				}

				if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
				{
					if (dir.is_directory())
					{
						Statics::SetSelectedDirectories({ dir.path() });
						Statics::SetSelectedFiles({});
						return;
					}
					else if (IsFileOfType(dir.path().filename().string(), ".bscn"))
					{
						Statics::OpenScene(dir.path().string());
					} else
					{
						LaunchExplorer(dir.path().string());
					}
				}

				if (ImGui::IsItemHovered()) mIsAnyFileHovered = true;

				if (ImGui::IsItemHovered() && (ImGui::IsMouseClicked(0) || ImGui::IsMouseClicked(1)) &&
					!(based::input::Keyboard::Key(BASED_INPUT_KEY_LSHIFT)
						|| based::input::Keyboard::Key(BASED_INPUT_KEY_LCTRL)))
				{
					ImGui::SetNextWindowFocus();
					if (!IsFileSelected(dir.path()) || Statics::GetSelectedFiles().size() > 1)
						Statics::SetSelectedFiles({ dir.path() });
					Statics::SetSelectedEntities({});
					mRenamePath = "";
				}
				else if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(0)
					&& based::input::Keyboard::Key(BASED_INPUT_KEY_LCTRL))
				{
					if (!IsFileSelected(dir.path()))
					{
						Statics::AddSelectedFile(dir.path());
					}
					else
					{
						Statics::RemoveSelectedFile(dir.path());
					}
				}
				else if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0)
					&& based::input::Keyboard::Key(BASED_INPUT_KEY_LSHIFT))
				{
					if (mFileMultiSelectBegin == -1)
					{
						Statics::SetSelectedFiles({ dir.path() });
						mFileMultiSelectBegin = mCurrentFileIndex;
					} else
					{
						mFileMultiSelectEnd = mCurrentFileIndex;
					}
					
					Statics::SetSelectedEntities({});
				}

				if (mRenamePath == dir.path())
				{
					std::string buffer = dir.path().filename().string();
					ImGuiInputTextFlags textFlags =
						ImGuiInputTextFlags_EnterReturnsTrue |
						ImGuiInputTextFlags_AutoSelectAll;
					ImGui::SetNextItemWidth(itemSize.x);
					if (ImGui::InputText("", &buffer, textFlags))
					{
						if (!buffer.empty())
						{
							auto fileType = dir.path().extension();
							auto newPath = fileType == "" ? path / buffer
								: path / (buffer + fileType.string());
							std::filesystem::rename(dir.path(), newPath);
							mRenamePath = "";
						}
					}
					ImGui::SetItemDefaultFocus();
				} else
				{
					ImGuiSelectableFlags flags = ImGuiSelectableFlags_AllowDoubleClick;
					bool selected = false;
					if (Statics::SelectedFilesContains(dir.path()))
						selected = true;

					ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, ImVec2(0.5f, 0.5f));
					ImGui::Selectable(dir.path().filename().string().c_str(), 
						selected, flags, ImVec2(itemSize.x, 0));
					ImGui::PopStyleVar();

					if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
					{
						mRenamePath = dir.path();
					} else if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0))
					{
						mRenamePath = "";
					}
				}

				ImGui::NextColumn();
			}
		}
		ImGui::Columns(1);
	}

	void FileBrowser::DrawContextMenu()
	{
		if (ImGui::BeginPopup("FilesContext"))
		{
			if (ImGui::BeginMenu("Create"))
			{
				if (ImGui::MenuItem("Folder", nullptr))
				{
					CreateObject(BasedFileType::Folder);
				}
				if (ImGui::MenuItem("Material", nullptr))
				{
					CreateObject(BasedFileType::Material);
				}
				if (ImGui::BeginMenu("UI"))
				{
					if (ImGui::MenuItem("UI Layout", nullptr))
					{
						CreateObject(BasedFileType::UIDoc);
					}
					ImGui::MenuItem("UI Style", nullptr);
					ImGui::EndMenu();
				}
				ImGui::EndMenu();
			}

			if (ImGui::MenuItem("Delete", nullptr))
			{
				auto result = tinyfd_messageBox(
					"Are you sure?",
					"This will delete these items permanently.",
					"yesno",
					"warning",
					0
				);
				if (result == 0)
				{
					ImGui::CloseCurrentPopup();
					ImGui::EndPopup();
					return;
				}

				for (auto& file : Statics::GetSelectedFiles())
					std::filesystem::remove(file);
				ImGui::CloseCurrentPopup();
			}

			if (ImGui::MenuItem("Show in Explorer", nullptr))
			{
				for (auto& dir : Statics::GetSelectedDirectories())
					LaunchExplorer(dir.string());
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
	}

	uint32_t FileBrowser::GetIconByFileType(const std::string& file)
	{
		if (file.find(".png") != std::string::npos
			|| file.find(".PNG") != std::string::npos
			|| file.find(".jpg") != std::string::npos
			|| file.find(".JPG") != std::string::npos
			|| file.find(".jpeg") != std::string::npos
			|| file.find(".JPEG") != std::string::npos
			|| file.find(".tga") != std::string::npos
			|| file.find(".TGA") != std::string::npos
			|| file.find(".ico") != std::string::npos
			|| file.find(".ICO") != std::string::npos)
		{
			return mImageIcon->GetId();
		}
		else if (file.find(".banim") != std::string::npos)
		{
			return mAnimationIcon->GetId();
		}
		else if (file.find(".bmat") != std::string::npos)
		{
			return mMaterialIcon->GetId();
		} 
		else if (file.find(".bscn") != std::string::npos)
		{
			return mSceneIcon->GetId();
		}
		else if (file.find(".dae") != std::string::npos
			|| file.find(".DAE") != std::string::npos
			|| file.find(".fbx") != std::string::npos
			|| file.find(".FBX") != std::string::npos
			|| file.find(".obj") != std::string::npos
			|| file.find(".OBJ") != std::string::npos
			|| file.find(".glTF") != std::string::npos
			|| file.find(".glb") != std::string::npos
			|| file.find(".3ds") != std::string::npos
			|| file.find(".3DS") != std::string::npos)
		{
			return m3DIcon->GetId();
		}
		else if (file.find(".wav") != std::string::npos
			|| file.find(".WAV") != std::string::npos
			|| file.find(".mp3") != std::string::npos
			|| file.find(".MP3") != std::string::npos
			|| file.find(".ogg") != std::string::npos
			|| file.find(".OGG") != std::string::npos
			|| file.find(".bank") != std::string::npos
			|| file.find(".fsb") != std::string::npos
			|| file.find(".flac") != std::string::npos)
		{
			return mAudioIcon->GetId();
		}
		else if (file.find(".rml") != std::string::npos)
		{
			return mUIIcon->GetId();
		}
		else if (file.find(".rcss") != std::string::npos)
		{
			return mStyleIcon->GetId();
		}
		else if (file.find(".") != std::string::npos)
		{
			return mFileIcon->GetId();
		}

		return mFolderIcon->GetId();
	}

	bool FileBrowser::InstantiateFile(const std::string& path)
	{
		for (Importer* importer : mImporters)
		{
			if (!importer->CanHandleFile(path)) continue;

			importer->HandleImport(path);
			return true;
		}

		return false;
	}

	bool FileBrowser::CreateObject(const editor::BasedFileType& type)
	{
		if (Statics::GetSelectedDirectories().empty()) return false;

		auto scene = based::Engine::Instance().GetApp().GetCurrentScene();
		auto path = std::filesystem::canonical(Statics::GetSelectedDirectories()[0]);
		bool res;

		// ALWAYS REASSIGN path FOR FILE SELECTION
		switch (type)
		{
		case BasedFileType::Folder:
			res = std::filesystem::create_directory(path / "New Folder");

			path = std::filesystem::canonical(path / "New Folder");
			break;
		case BasedFileType::Material:
			{
			std::ifstream ifs("Assets/Templates/MaterialTemplate.txt");
			std::stringstream strStream;
			strStream << ifs.rdbuf();

			auto materialName = scene->GetMaterialStorage().GetSafeName("Lit");
			based::core::UUID id;

			std::string output = strStream.str();
			output.replace(output.find("__MATERIAL_NAME__"),
				std::string("__MATERIAL_NAME__").length(),
				materialName);
			output.replace(output.find("__MATERIAL_UUID__"),
				std::string("__MATERIAL_UUID__").length(),
				std::to_string(id));

			std::ofstream ofs(Statics::GetSelectedDirectories()[0].string() + "/" + materialName + ".bmat");
			ofs << output;
			ofs.close();

			path = std::filesystem::canonical(Statics::GetSelectedDirectories()[0].string()
				+ "/" + materialName + ".bmat");

			auto newMaterial =
				based::graphics::Material::LoadMaterialWithUUID(
					path.string(), id, Statics::GetSelectedDirectories()[0].string() + "/", true);

			if (newMaterial) res = true;
			else
			{
				std::filesystem::remove(path);
				res = false;
			}
			break;
			}
		case BasedFileType::UIDoc:
			{
			std::ifstream ifs("Assets/Templates/UIDocTemplate.txt");
			std::stringstream strStream;
			strStream << ifs.rdbuf();
			std::string output = strStream.str();

			std::filesystem::path rmlPath; // IGNORED
			if (!DoesProjectContainFile("rml.rcss", rmlPath))
			{
				auto selection = tinyfd_messageBox(
					"Import Default RML Styles?",
					"Would you like to import the default RCSS style sheet?",
					"yesno",
					"question",
					0
				);
				if (selection == 1)
				{
					std::ifstream ifs("Assets/Templates/rml.txt");
					std::stringstream strStream;
					strStream << ifs.rdbuf();
					std::string output = strStream.str();
					std::ofstream ofs(Statics::GetSelectedDirectories()[0].string() + "/rml.rcss");
					ofs << output;
					ofs.close();
				}
			}

			std::string baseFileName = "New UI Doc";
			std::string fileName = baseFileName;
			int index = 1;
			while (std::filesystem::exists(path / (fileName + ".rml")))
			{
				fileName = baseFileName + std::to_string(index);
				index++;
			}

			std::ofstream ofs(Statics::GetSelectedDirectories()[0].string() + "/" + fileName + ".rml");
			ofs << output;
			ofs.close();

			path = std::filesystem::canonical(Statics::GetSelectedDirectories()[0] / (fileName + ".rml"));

			res = true;
			break;
			}
		case BasedFileType::UIStyle:
			{
			std::ifstream ifs("Assets/Templates/UIStyleTemplate.txt");
			std::stringstream strStream;
			strStream << ifs.rdbuf();
			std::string output = strStream.str();

			std::filesystem::path rmlPath; // IGNORED
			if (!DoesProjectContainFile("rml.rcss", rmlPath))
			{
				auto selection = tinyfd_messageBox(
					"Import Default RML Styles?",
					"Would you like to import the default RCSS style sheet?",
					"yesno",
					"question",
					0
				);
				if (selection == 1)
				{
					std::ifstream ifs("Assets/Templates/rml.txt");
					std::stringstream strStream;
					strStream << ifs.rdbuf();
					std::string output = strStream.str();
					std::ofstream ofs(Statics::GetSelectedDirectories()[0].string() + "/rml.rcss");
					ofs << output;
					ofs.close();
				}
			}

			std::string baseFileName = "New UI Style";
			std::string fileName = baseFileName;
			int index = 1;
			while (std::filesystem::exists(path / (fileName + ".rcss")))
			{
				fileName = baseFileName + std::to_string(index);
				index++;
			}

			std::ofstream ofs(Statics::GetSelectedDirectories()[0].string() + "/" + fileName + ".rcss");
			ofs << output;
			ofs.close();

			path = std::filesystem::canonical(Statics::GetSelectedDirectories()[0] / (fileName + ".rcss"));

			res = true;
			break;
			}
		default:
			BASED_WARN("Unhandled file type was passed to CreateObject! (filebrowser.cpp)");
		}

		if (res)
		{
			mRenamePath = path;
			mScrollTarget = path;
		}

		return res;
	}
}
