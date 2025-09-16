#include <ktx.h>
#include <string>
#include <filesystem>
#include <queue>
#include <thread>

//#include "profiler.h"
#define PROFILE_FUNCTION()

namespace fs = std::filesystem;

#define FLAG_IMPLEMENTATION
#include "flag.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize2.h"

#include "jobmanager.h"

namespace
{
    fs::path destination_path;
    fs::path source_path;
    based::managers::JobManager* job_manager;

    bool* help = flag_bool("help", false, "Display cli usage flags.");
    char** project_directory = flag_str("i", "", "Root directory of the project to build.");
    char** config = flag_str("c", "Debug", "Debug or Release.");
    //bool* multithread = flag_bool("mt", false, "Run multithreaded.");
    bool* generate_mips = flag_bool("gen-mipmaps", false, "Automatically generate mipmaps by "
                                           "downsampling the input texture.");
    bool* no_transcode = flag_bool("no-transcode", false, "Prevent transcoding into GPU-specific formats."
                                                          " This will force the engine to do this at runtime instead.");
    char** override_name = flag_str("override-name", "",
                                           "Name to use when outputting baked assets and other built files, "
                                           "in case the default is incorrect.\n"
                                           "I.e. -override-name MyProj will output to "
                                           "ActualProjectRoot/bin/{config}/MyProj/");
    char** target = flag_str("t", "desktop", "Target platform. Determines which texture compression"
                                             " algorithm to use, for compatibility reasons.\n"
                                             "Options are: desktop, legacy-desktop, web, ios, android");
    Flag_List* normal_id = flag_list("normal-identifiers",
                                                    "String to identify which textures are normal maps. "
                                                    "\nDefault: normal (non-case sensitive).");

    // TODO: Get rid of this
    int files = 0;

    void usage(FILE* stream)
    {
        fprintf(stream, "Usage: ./BasedBuildTool -i <PROJECT DIRECTORY> [OPTIONS] <OUTPUT FILES...>\n");
        fprintf(stream, "OPTIONS:\n");
        flag_print_options(stream);
    }

    std::string get_project_name(const fs::path& path, char** override_name)
    {
        if (*override_name && strcmp(*override_name, "") != 0)
        {
            return std::string(*override_name);
        }

        return path.filename().string();
    }

    fs::path make_output_path(const fs::path& inputPath,
                              const fs::path& projectRoot,
                              const fs::path& destinationRoot,
                              const std::string& extensionOverride = "")
    {
        fs::path relative = fs::relative(inputPath, projectRoot);

        fs::path output_path = destinationRoot / relative;
        if (relative.string().find("PostBuildCopy") != std::string::npos)
        {
            fs::path temp;
            for (const auto& component : output_path) {
                if (component.string() != "PostBuildCopy" && component.string() != "PostBuildCopy_windows") {
                    temp /= component;
                }
            }
            output_path = temp;
        }

        if (!extensionOverride.empty())
        {
            output_path.replace_extension(extensionOverride);
        }

        return output_path;
    }

    bool is_normal_map(const fs::path& path)
    {
        PROFILE_FUNCTION();
        
        std::string filename = path.filename().string();
        std::transform(filename.begin(), filename.end(),
            filename.begin(), ::tolower);
        
        if (normal_id->count > 0)
        {
            for (size_t i = 0; i < normal_id->count; ++i)
            {
                const char* identifier = normal_id->items[i];
                if (filename.find(identifier) != std::string::npos) return true;
            }
        } else
        {
            for (const std::string& identifier : { "normal" })
            {
                if (filename.find(identifier) != std::string::npos) return true;
            }
        }

        return false;
    }

    ktx_transcode_fmt_e get_transcode_format(const fs::path& path, bool is_normal_map, uint32_t channels)
    {
        PROFILE_FUNCTION();
        
        if (strcmp(*target, "ios") == 0)
        {
            return KTX_TTF_PVRTC1_4_RGBA;
        }
        else if (strcmp(*target, "android") == 0)
        {
            return KTX_TTF_ETC2_RGBA;
        }
        else if (strcmp(*target, "web") == 0)
        {
            return KTX_TTF_BC3_RGBA;
        }
        else
        {
            if (is_normal_map)
                return strcmp(*target, "legacy-desktop") == 0
                            ? KTX_TTF_BC3_RGBA // Use BC3 for supporting older GPUs
                            : KTX_TTF_BC5_RG;
            else if (channels == 1) return KTX_TTF_BC4_R;
            else return strcmp(*target, "legacy-desktop") == 0
                            ? KTX_TTF_BC3_RGBA // Use BC3 for supporting older GPUs
                            : KTX_TTF_BC7_RGBA;   
        }
    }

    std::string get_transcode_name(ktx_transcode_fmt_e format)
    {
        if (format == KTX_TTF_PVRTC1_4_RGBA) return "PVRTC 4bit";
        if (format == KTX_TTF_ETC1_RGB)      return "ETC1";
        if (format == KTX_TTF_ETC2_RGBA)     return "ETC2";
        if (format == KTX_TTF_BC1_RGB)       return "BC1";
        if (format == KTX_TTF_BC3_RGBA)      return "BC3";
        if (format == KTX_TTF_BC4_R)         return "BC4";
        if (format == KTX_TTF_BC5_RG)        return "BC5";
        if (format == KTX_TTF_BC7_RGBA)      return "BC7";

        return "Unknown Format";
    }

    stbi_uc* prepare_normal_map(const stbi_uc* src,
                                      int width, int height, int& channels)
    {
        PROFILE_FUNCTION();
        
        stbi_uc* dst = (stbi_uc*)malloc(width * height * 4 * sizeof(stbi_uc));

        for (int i = 0; i < width * height; ++i) {
            stbi_uc r = src[i * channels + 0]; // X
            stbi_uc g = src[i * channels + 1]; // Y
            stbi_uc b = channels > 2 ? src[i * channels + 2] : 0;

            dst[i * 4 + 0] = r;   // R = X
            dst[i * 4 + 1] = g;   // G = Y (keep for debugging/consistency)
            dst[i * 4 + 2] = b;   // B = Z (ignored by BC5)
            dst[i * 4 + 3] = g;   // A = Y → used as BC5.G
        }

        channels = 4;
        return dst;
    }

    bool handle_texture_compression(const fs::path& path)
    {
        PROFILE_FUNCTION();
        //PROFILE_SCOPE_TEXT("Compressing texture: %s", path.filename().string().c_str());
        
        if (!is_regular_file(path)) return false;
        
        fs::path output_path = make_output_path(path, source_path, destination_path, ".ktx2");

        // Skip compressing files that already exist and are newer than their original input
        if (fs::exists(output_path) && fs::last_write_time(path) < fs::last_write_time(output_path))
        {
            printf("Target %s is up to date, skipping.\n", output_path.filename().string().c_str());
            return true;
        }

        const bool texture_is_normal_map = is_normal_map(path.string());

        auto now = std::chrono::system_clock::now();
        auto start = now.time_since_epoch();

        int width, height, channels;
        if (stbi_uc* data = stbi_load(path.string().c_str(), &width, &height, &channels, 0))
        {
            // Handle compressing images to KTX2
            printf("Processing: %s\n", path.filename().string().c_str());

            // Normal maps must copy the G channel to the A channel, otherwise
            // libktx just fills G in with 255 (see ktx_transcode_fmt_e documentation)
            if (texture_is_normal_map)
            {
                stbi_uc* original_data = data;
                data = prepare_normal_map(data, width, height, channels);
                stbi_image_free(original_data);
            }

            ktxTexture2* texture;
            ktxTextureCreateInfo createInfo = {};
            if (channels == 1) createInfo.vkFormat = 9; // VK_FORMAT_R8_UNORM
            else if (channels == 3) createInfo.vkFormat = 23; // VK_FORMAT_R8G8B8_UNORM
            else createInfo.vkFormat = 37; // VK_FORMAT_R8G8B8A8_UNORM
            //createInfo.vkFormat = 37;
            createInfo.baseWidth = width;
            createInfo.baseHeight = height;
            createInfo.baseDepth = 1;
            createInfo.numDimensions = 2;
            createInfo.numLevels = *generate_mips
                                       ? 1u + static_cast<uint32_t>(floor(log2(std::max(width, height))))
                                       : 1;
            createInfo.numLayers = 1;
            createInfo.numFaces = 1;
            createInfo.isArray = KTX_FALSE;
            // If we didn't pre-generate mips then we'll need to do it when uploading to the GPU
            createInfo.generateMipmaps = !*generate_mips; 

            ktx_error_code_e result = ktxTexture2_Create(&createInfo,
                                                         KTX_TEXTURE_CREATE_ALLOC_STORAGE, &texture);
            if (result != KTX_SUCCESS)
            {
                fprintf(stderr, "   Failed to create texture: %d\n", result);
                stbi_image_free(data);
                return false;
            }

            result = ktxTexture_SetImageFromMemory(ktxTexture(texture), 0, 0, 0,
                                                   data, width * height * channels);
            if (result != KTX_SUCCESS)
            {
                fprintf(stderr, "   Failed to set image: %d\n", result);
                ktxTexture2_Destroy(texture);
                stbi_image_free(data);
                return false;
            }

            std::vector<std::vector<stbi_uc>> mip_list;
            int prev_width = width;
            int prev_height = height;

            for (uint32_t level = 1; level < texture->numLevels; ++level)
            {
                int mip_width = std::max(1, prev_width / 2);
                int mip_height = std::max(1, prev_height / 2);

                stbi_uc* last_mip_pixels = mip_list.empty() ? data : mip_list.back().data();
                mip_list.emplace_back(mip_width * mip_height * channels);
                std::vector<stbi_uc>& mip_pixels = mip_list.back();

                stbir_resize_uint8_srgb(
                    last_mip_pixels, prev_width, prev_height, 0,
                    mip_pixels.data(), mip_width, mip_height, 0,
                    static_cast<stbir_pixel_layout>(channels)
                );

                result = ktxTexture_SetImageFromMemory(ktxTexture(texture), level, 0, 0, mip_pixels.data(),
                                                       mip_pixels.size());
                if (result != KTX_SUCCESS)
                {
                    fprintf(stderr, "   Failed to generate mip level %u for %s\n",
                        level, path.filename().string().c_str());
                    ktxTexture2_Destroy(texture);
                    stbi_image_free(data);
                    return false;
                }

                prev_width = mip_width;
                prev_height = mip_height;
            }

            ktxBasisParams basisParams = {0};
            basisParams.structSize = sizeof(basisParams);
            basisParams.threadCount = std::thread::hardware_concurrency() - 1u;
            basisParams.uastcFlags = KTX_PACK_UASTC_LEVEL_DEFAULT;
            basisParams.uastc = KTX_TRUE;

            printf("    Compressing with %u threads.\n", basisParams.threadCount);

            if (!createInfo.generateMipmaps)
            {
                result = ktxTexture2_CompressBasisEx(texture, &basisParams);
                if (result != KTX_SUCCESS)
                {
                    fprintf(stderr, "   Failed to compress %s to UASTC: %d\n", path.filename().string().c_str(), result);
                    ktxTexture2_Destroy(texture);
                    stbi_image_free(data);
                    return false;
                }
            }

            ktx_transcode_fmt_e transcode_format = get_transcode_format(path, texture_is_normal_map, channels);
            printf("    Transcoding to %s\n", get_transcode_name(transcode_format).c_str());

            if (!*no_transcode && !createInfo.generateMipmaps)
            {
                result = ktxTexture2_TranscodeBasis(texture,
                                                transcode_format,
                                                KTX_TF_HIGH_QUALITY);
                if (result != KTX_SUCCESS)
                {
                    fprintf(stderr, "   Failed to transcode to %s: %d\n",
                        get_transcode_name(transcode_format).c_str(), result);
                    ktxTexture2_Destroy(texture);
                    stbi_image_free(data);
                    return false;
                }
            }

            if (!fs::exists(output_path.parent_path()))
            {
                printf("    Directory %s does not exist! Creating it now.\n", output_path.parent_path().string().c_str());
                fs::create_directories(output_path.parent_path());
            }

            result = ktxTexture2_WriteToNamedFile(texture, output_path.string().c_str());
            if (result != KTX_SUCCESS)
            {
                fprintf(stderr, "   Failed to write KTX2 file: %d\n", result);
                ktxTexture2_Destroy(texture);
                stbi_image_free(data);
                return false;
            }

            now = std::chrono::system_clock::now();
            auto end = now.time_since_epoch();
            auto duration = end - start;

            auto seconds = std::chrono::duration<double>(duration).count();

            printf("Successfully wrote file: %s in %.2fs\n", output_path.filename().string().c_str(), seconds);
            
            ktxTexture_Destroy(ktxTexture(texture));
            stbi_image_free(data);
            files++;
            return true;
        }

        return false;
    }

    bool handle_copy(const fs::path& source, const fs::path& dest)
    {
        PROFILE_FUNCTION();
        
        if (!is_regular_file(source)) return false;

        printf("Copying %s to %s\n", source.filename().string().c_str(), dest.string().c_str());
        
        std::string filename_str = source.filename().string();
        
#ifdef BASED_PLATFORM_WINDOWS

        if (!fs::exists(dest.parent_path())) fs::create_directories(dest.parent_path());
        
        std::string copy_cmd = "robocopy ";
        copy_cmd += "\"" + source.parent_path().string() + "\" ";
        copy_cmd += "\"" + dest.parent_path().string() + "\" ";
        copy_cmd += "\"" + filename_str + "\" ";
        copy_cmd += "/mt ";
        
#else
        std::string copy_cmd = "cp " + source_str + " " + dest_str;
#endif

        int result = fflush(stdout);
        std::system(copy_cmd.c_str());
        result = fflush(stdout);
        
        return true; // There are a lot of different return codes, and this is run last anyway
    }

    bool handle_file(const fs::path& path)
    {
        PROFILE_FUNCTION();
        
        bool handled = handle_texture_compression(path);
        if (!handled)
            handled = handle_copy(path, make_output_path(path, source_path, destination_path));

        return handled;
    }
}

int main(int argc, char* argv[])
{
    if (!flag_parse(argc, argv)) {
        usage(stderr);
        flag_print_error(stderr);
        exit(1);
    }

    if (*help) {
        usage(stdout);
        exit(0);
    }

    if (!*project_directory || strcmp(*project_directory, "") == 0)
    {
        usage(stderr);
        fprintf(stderr, "Must specify a project directory with -i!\n");
        exit(1);
    }

    source_path = fs::path(*project_directory);
    if (!fs::is_directory(source_path))
    {
        usage(stderr);
        fprintf(stderr, "Project directory must be a directory!\n");
        exit(1);
    }

    if (!*config || !(strcmp(*config, "Debug") == 0 || strcmp(*config, "Release") == 0))
    {
        usage(stderr);
        fprintf(stderr, "Must specify a valid build configuration (Debug or Release) with -c!\n");
        exit(1);
    }

    /*if (*multithread)
    {
        job_manager = new based::managers::JobManager();
        job_manager->Initialize();
        printf("Running multithreaded with %u threads.\n", job_manager->GetNumThreads());
    }*/

    const std::string project_name = get_project_name(source_path, override_name);
    destination_path = source_path / "bin" / *config / project_name;

    if (!fs::exists(destination_path))
    {
        printf("Output directory %s does not exist! Creating it now.\n", destination_path.string().c_str());
        fs::create_directories(destination_path);
    }

    std::queue<fs::path> paths_to_search;
#ifdef BASED_PLATFORM_WINDOWS
    paths_to_search.push(source_path / "PostBuildCopy_windows");
#else
    paths_to_search.push(source_path / "PostBuildCopy");
#endif
    paths_to_search.push(source_path / "Assets");

    auto now = std::chrono::system_clock::now();
    auto start = now.time_since_epoch();
    
    while (!paths_to_search.empty())
    {
        fs::path curPath = paths_to_search.front();
        paths_to_search.pop();

        for (const auto& file : fs::directory_iterator(curPath))
        {
            if (fs::is_directory(file.path()))
            {
                paths_to_search.push(file.path());
                continue;
            }

            if (fs::is_regular_file(file.path()))
            {
                handle_file(file);
            }
        }
    }

    now = std::chrono::system_clock::now();
    auto end = now.time_since_epoch();
    auto duration = end - start;

    auto minutes = std::chrono::duration_cast<std::chrono::minutes>(duration);
    duration -= minutes;
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
    duration -= seconds;
    auto milliseconds = duration;

    printf("Processed %i files in %dm:%llds:%lldms", files, minutes.count(), seconds.count(), milliseconds.count());

    int rest_argc = flag_rest_argc();
    char **rest_argv = flag_rest_argv();

    // Cleanup

    /*if (*multithread)
    {
        job_manager->Shutdown();
        delete job_manager;
    }*/
}