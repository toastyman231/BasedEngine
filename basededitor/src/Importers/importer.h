#pragma once

namespace based::graphics
{
	class Mesh;
}

namespace editor
{
	class Importer
	{
	public:
		Importer() = default;
		virtual ~Importer() = default;

		virtual bool CanHandleFile(const std::string& path) = 0;

		virtual void HandleImport(const std::string& path) = 0;

	protected:
		static bool IsFileOfType(const std::string& path, const std::string& type)
		{
			return path.find(type) != std::string::npos;
		}
	};

	class MeshImporter : public Importer
	{
	public:
		MeshImporter() = default;

		bool CanHandleFile(const std::string& path) override;
		void HandleImport(const std::string& path) override;
	};

	class ModelImporter : public Importer
	{
	public:
		ModelImporter() = default;

		bool CanHandleFile(const std::string& path) override;
		void HandleImport(const std::string& path) override;
	};

	class TextureImporter : public Importer
	{
	public:
		TextureImporter() = default;

		bool CanHandleFile(const std::string& path) override;
		void HandleImport(const std::string& path) override;
	};

	class MaterialImporter : public Importer
	{
	public:
		MaterialImporter() = default;

		bool CanHandleFile(const std::string& path) override;
		void HandleImport(const std::string& path) override;
	};

	class AnimationImporter : public Importer
	{
	public:
		AnimationImporter() = default;

		bool CanHandleFile(const std::string& path) override;
		void HandleImport(const std::string& path) override;
	};
}
