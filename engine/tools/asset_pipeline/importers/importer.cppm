module;

#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <cctype>

export module tools.asset_pipeline.importer;

import core.stdtypes;
import tools.asset_pipeline.types.mesh_types;

export namespace draco::tools::assets
{
    enum class AssetResult : u8
    {
        Success = 0,
        FileNotFound,
        InvalidFormat,
        ParserError,
        CookError
    };

    class IImporter
    {
    public:
        virtual ~IImporter() = default;

        virtual AssetResult import_mesh(const std::filesystem::path& path, ImportedMesh& out_mesh) = 0;
    };

    class ImporterRegistry
    {
    public:
        static ImporterRegistry& get()
        {
            static ImporterRegistry instance;
            return instance;
        }

        void register_importer(const std::string& extension, std::shared_ptr<IImporter> importer)
        {
            std::string ext = format_extension(extension);
            m_importers[ext] = std::move(importer);
        }

        std::shared_ptr<IImporter> get_importer(const std::string& extension)
        {
            std::string ext = format_extension(extension);

            auto it = m_importers.find(ext);
            if (it != m_importers.end())
                return it->second;

            return nullptr;
        }

        AssetResult import_file(const std::filesystem::path& path, ImportedMesh& out_mesh)
        {
            if (!std::filesystem::exists(path))
                return AssetResult::FileNotFound;

            std::string ext = format_extension(path.extension().string());

            auto importer = get_importer(ext);
            if (!importer)
                return AssetResult::InvalidFormat;

            return importer->import_mesh(path, out_mesh);
        }

    private:
        ImporterRegistry() = default;

        std::string format_extension(const std::string& ext)
        {
            std::string lower = ext;

            std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) { return std::tolower(c); });

            if (!lower.empty() && lower[0] == '.')
                return lower.substr(1);

            return lower;
        }

        std::unordered_map<std::string, std::shared_ptr<IImporter>> m_importers;
    };
}