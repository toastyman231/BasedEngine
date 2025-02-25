#include "based/pch.h"
#include "filebrowser.h"

#include "based/input/keyboard.h"
#include "external/imgui/imgui.h"
#include "external/imgui/imgui_internal.h"

namespace editor::panels
{
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
			DrawFileViewer();
		}
		ImGui::End();
	}

	bool FileBrowser::IsDirectorySelected(const std::string& path)
	{
		return std::find(mSelectedDirectories.begin(), mSelectedDirectories.end(), path) != mSelectedDirectories.end();
	}

	bool FileBrowser::IsFileSelected(const std::string& path)
	{
		return std::find(mSelectedFiles.begin(), mSelectedFiles.end(), path) != mSelectedFiles.end();
	}

	bool FileBrowser::IsDirectoryLeaf(const std::string& path)
	{
		for (auto dir : std::filesystem::directory_iterator(path))
		{
			if (dir.is_directory()) return false;
		}

		return true;
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
				if (dir.is_directory() && IsDirectoryLeaf(dir.path().string())) 
					flags |= ImGuiTreeNodeFlags_Leaf;
				if (IsDirectorySelected(dir.path().string())) 
					flags |= ImGuiTreeNodeFlags_Selected;

				if (!dir.is_directory()) continue;

				++mCurrentDirIndex;

				if (!mSelectedDirectories.empty())
				{
					if (mSelectedDirectories.back() == dir.path().string())
						mDirMultiSelectBegin = mCurrentDirIndex;
				}

				int32_t min = std::min(mDirMultiSelectBegin, mDirMultiSelectEnd);
				int32_t max = std::max(mDirMultiSelectBegin, mDirMultiSelectEnd);
				mDirCountMax = max - min;

				if (mDirMultiSelectEnd != -1 && mCurrentDirIndex >= min
					&& mCurrentDirIndex <= max)
				{
					if (!IsDirectorySelected(dir.path().string()))
						mSelectedDirectories.emplace_back(dir.path().string());
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
					mSelectedDirectories = { dir.path().string() };
				}
				else if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(0)
					&& based::input::Keyboard::Key(BASED_INPUT_KEY_LCTRL))
				{

					if (!IsDirectorySelected(dir.path().string()))
					{
						mSelectedDirectories.emplace_back(dir.path().string());
					}
					else
					{
						mSelectedDirectories.erase(
							std::find(mSelectedDirectories.begin(), mSelectedDirectories.end(),
								dir.path().string()));
					}
				}
				else if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0)
					&& based::input::Keyboard::Key(BASED_INPUT_KEY_LSHIFT))
				{
					mDirMultiSelectEnd = mCurrentDirIndex;
				}

				if (dir.is_directory())
					if (isOpen) DrawDirectoryTree(dir.path().string());

				if (isOpen) ImGui::TreePop();
			}
		}
	}

	void FileBrowser::DrawFileViewer()
	{
		ImVec2 itemSize = { 100.f, 100.f };
		int numColumns = (int)std::floor(ImGui::GetContentRegionAvail().x / itemSize.x);
		ImGui::Columns(numColumns, 0, false);

		mCurrentFileIndex = 0;
		mCurrentFileCount = 0;
		for (auto& p : mSelectedDirectories)
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
				mCurrentFileIndex++;
				ImVec2 pos = ImGui::GetCursorScreenPos();

				if (!mSelectedFiles.empty())
				{
					if (mSelectedFiles.back() == dir.path().string())
						mFileMultiSelectBegin = mCurrentFileIndex;
				}

				int32_t min = std::min(mFileMultiSelectBegin, mFileMultiSelectEnd);
				int32_t max = std::max(mFileMultiSelectBegin, mFileMultiSelectEnd);
				mFileCountMax = max - min;

				if (mFileMultiSelectEnd != -1 && mCurrentFileIndex >= min
					&& mCurrentFileIndex <= max)
				{
					if (!IsFileSelected(dir.path().string()))
						mSelectedFiles.emplace_back(dir.path().string());
					mCurrentFileCount++;

					if (mCurrentFileCount >= mFileCountMax + 1)
					{
						mFileMultiSelectBegin = -1;
						mFileMultiSelectEnd = -1;
					}
				}

				bool selected = IsFileSelected(dir.path().string());
				ImGui::Selectable(
					"",
					&selected, ImGuiSelectableFlags_AllowDoubleClick,
					itemSize);
				ImGui::SetItemAllowOverlap();

				ImGui::SetCursorScreenPos({ pos.x - 4, pos.y });
				ImGui::Image(
					(void*)static_cast<intptr_t>(GetIconByFileType(dir.path().filename().string())),
					itemSize, { 0, 1 }, { 1, 0 });

				if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
				{
					if (dir.is_directory())
					{
						mSelectedDirectories = { dir.path().string() };
					}
				}

				if (ImGui::IsItemHovered() && (ImGui::IsMouseClicked(0) || ImGui::IsMouseClicked(1)) &&
					!(based::input::Keyboard::Key(BASED_INPUT_KEY_LSHIFT)
						|| based::input::Keyboard::Key(BASED_INPUT_KEY_LCTRL)))
				{
					mSelectedFiles = { dir.path().string() };
				}
				else if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(0)
					&& based::input::Keyboard::Key(BASED_INPUT_KEY_LCTRL))
				{

					if (!IsFileSelected(dir.path().string()))
					{
						mSelectedFiles.emplace_back(dir.path().string());
					}
					else
					{
						mSelectedFiles.erase(
							std::find(mSelectedFiles.begin(), mSelectedFiles.end(),
								dir.path().string()));
					}
				}
				else if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0)
					&& based::input::Keyboard::Key(BASED_INPUT_KEY_LSHIFT))
				{
					mFileMultiSelectEnd = mCurrentFileIndex;
				}

				ImGui::Text(dir.path().filename().string().c_str());

				ImGui::NextColumn();
			}
		}
		ImGui::Columns(1);
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
}
