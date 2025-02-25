#pragma once
#include <string>

#include "panelbase.h"
#include "../editorstatics.h"

namespace editor::panels
{
	class FileBrowser : public Panel
	{
	public:
		explicit FileBrowser(const std::string& title)
			: Panel(title)
		{
			mFolderIcon = std::make_shared<based::graphics::Texture>("Assets/Icons/folder.png");
			mImageIcon = std::make_shared<based::graphics::Texture>("Assets/Icons/image.png");
			mAnimationIcon = std::make_shared<based::graphics::Texture>("Assets/Icons/animation.png");
			mMaterialIcon = std::make_shared<based::graphics::Texture>("Assets/Icons/material.png");
			mFileIcon = std::make_shared<based::graphics::Texture>("Assets/Icons/file.png");
			mSceneIcon = std::make_shared<based::graphics::Texture>("Assets/Icons/scene.png");
			m3DIcon = std::make_shared<based::graphics::Texture>("Assets/Icons/3d.png");
			mAudioIcon = std::make_shared<based::graphics::Texture>("Assets/Icons/audio.png");
			mUIIcon = std::make_shared<based::graphics::Texture>("Assets/Icons/ui.png");
			mStyleIcon = std::make_shared<based::graphics::Texture>("Assets/Icons/style.png");
		}

		~FileBrowser() override = default;

		void Initialize() override {}
		void Update(float deltaTime) override {}
		void Render() override;
		void Shutdown() override {}

		bool HandleFileDrop(const std::string& path);

		bool IsDirectorySelected(const std::string& path);
		bool IsFileSelected(const std::string& path);
		bool IsDirectoryLeaf(const std::string& path);
		bool IsFileOfType(const std::string& path, const std::string& type);
		bool IsFileViewerHovered();

	private:
		void DrawDirectoryTree(const std::string& path);
		void DrawFileViewer();
		uint32_t GetIconByFileType(const std::string& file);

		std::vector<std::string> mExcludeDirs = 
			{ "bin", "bin-obj", "PostBuildCopy", "PostBuildCopy_windows",
			".git", ".vs", "ProjectFiles" };

		std::vector<std::string> mSelectedDirectories;
		std::vector<std::string> mSelectedFiles;

		std::shared_ptr<based::graphics::Texture> mFolderIcon;
		std::shared_ptr<based::graphics::Texture> mImageIcon;
		std::shared_ptr<based::graphics::Texture> mAnimationIcon;
		std::shared_ptr<based::graphics::Texture> mMaterialIcon;
		std::shared_ptr<based::graphics::Texture> mFileIcon;
		std::shared_ptr<based::graphics::Texture> mSceneIcon;
		std::shared_ptr<based::graphics::Texture> m3DIcon;
		std::shared_ptr<based::graphics::Texture> mAudioIcon;
		std::shared_ptr<based::graphics::Texture> mUIIcon;
		std::shared_ptr<based::graphics::Texture> mStyleIcon;

		bool mIsFileViewerHovered = false;

		int mCurrentDirIndex = 0;
		int mCurrentDirCount = 0;
		int mDirCountMax = 0;

		int mDirMultiSelectBegin = -1;
		int mDirMultiSelectEnd = -1;

		int mCurrentFileIndex = 0;
		int mCurrentFileCount = 0;
		int mFileCountMax = 0;

		int mFileMultiSelectBegin = -1;
		int mFileMultiSelectEnd = -1;
	};
}
