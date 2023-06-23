#include <node_api.h>
#include <gio/gio.h>
#include <string>
#include <vector>

std::vector<std::string> ls(const std::string& directoryPath) {
    std::vector<std::string> fileList;

    GError* error = nullptr;
    GFile* directory = g_file_new_for_path(directoryPath.c_str());
    GFileEnumerator* enumerator = g_file_enumerate_children(directory, "standard::name", G_FILE_QUERY_INFO_NONE, nullptr, &error);

    if (error != nullptr) {
        g_clear_error(&error);
        g_object_unref(directory);
        return fileList;
    }

    GFileInfo* fileInfo = nullptr;
    while ((fileInfo = g_file_enumerator_next_file(enumerator, nullptr, &error)) != nullptr) {
        const char* fileName = g_file_info_get_name(fileInfo);
        fileList.push_back(fileName);
        g_object_unref(fileInfo);
    }

    if (error != nullptr) {
        g_clear_error(&error);
    }

    g_object_unref(enumerator);
    g_object_unref(directory);

    return fileList;
}

napi_value ListDirectoryWrapper(napi_env env, napi_callback_info info) {
    napi_status status;

    size_t argc = 1;
    napi_value argv[1];
    status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (status != napi_ok || argc < 1) {
        napi_throw_error(env, nullptr, "Invalid arguments");
        return nullptr;
    }

    size_t pathSize;
    status = napi_get_value_string_utf8(env, argv[0], nullptr, 0, &pathSize);
    if (status != napi_ok) {
        napi_throw_error(env, nullptr, "Invalid directory path");
        return nullptr;
    }

    std::vector<char> pathBuffer(pathSize + 1);
    status = napi_get_value_string_utf8(env, argv[0], pathBuffer.data(), pathSize + 1, nullptr);
    if (status != napi_ok) {
        napi_throw_error(env, nullptr, "Invalid directory path");
        return nullptr;
    }

    std::string directoryPath(pathBuffer.data());
    std::vector<std::string> fileList = ls(directoryPath);

    napi_value result;
    status = napi_create_array_with_length(env, fileList.size(), &result);
    if (status != napi_ok) {
        napi_throw_error(env, nullptr, "Failed to create result array");
        return nullptr;
    }

    for (size_t i = 0; i < fileList.size(); i++) {
        napi_value fileName;
        status = napi_create_string_utf8(env, fileList[i].c_str(), NAPI_AUTO_LENGTH, &fileName);
        if (status != napi_ok) {
            napi_throw_error(env, nullptr, "Failed to create file name string");
            return nullptr;
        }

        status = napi_set_element(env, result, i, fileName);
        if (status != napi_ok) {
            napi_throw_error(env, nullptr, "Failed to set element in result array");
            return nullptr;
        }
    }

    return result;
}

napi_value Init(napi_env env, napi_value exports) {
    napi_status status;
    napi_value fn;

    status = napi_create_function(env, nullptr, 0, ListDirectoryWrapper, nullptr, &fn);
    if (status != napi_ok) {
        napi_throw_error(env, nullptr, "Failed to create function");
        return nullptr;
    }

    status = napi_set_named_property(env, exports, "listDirectory", fn);
    if (status != napi_ok) {
        napi_throw_error(env, nullptr, "Failed to set named property");
        return nullptr;
    }

    return exports;
}