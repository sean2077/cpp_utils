/// 文件系统相关工具函数
#pragma once

#ifdef _WIN32
#include <Shlwapi.h>
#include <direct.h>
#include <windows.h>
#else
#include <dirent.h>
#include <errno.h>
#include <glob.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#include <algorithm>
#include <codecvt>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

namespace {

// ==============================================================================================
//                                         文件路径
// ==============================================================================================

#ifdef _WIN32
constexpr char path_separator = '\\';
#else
constexpr char path_separator = '/';
#endif

/**
 * @brief Join one or more path components intelligently.
 *
 * @param paths One or more path components to join.
 * @return std::string The joined path.
 */
inline std::string path_join(const std::initializer_list<std::string> &paths) {
  std::string result;
  for (const auto &path : paths) {
    if (!result.empty() && result.back() != path_separator &&
        path.front() != path_separator) {
      result += path_separator;
    }
    result += path;
  }
  return result;
}

/**
 * @brief Join two or more paths together.
 *
 * @param path1 First path to join.
 * @param paths Other paths to join.
 * @return std::string The joined path.
 */
template <typename... Args>
inline std::string path_join(const std::string &path1, Args... paths) {
  return path_join({path1, paths...});
}

/**
 * @brief Make a directory recursively.
 *
 * @param path The directory path to create.
 * @return bool True if the directory was created successfully, false otherwise.
 */
inline bool makedirs(const std::string &path,
                     mode_t mode = S_IRWXU | S_IRWXG | S_IRWXO) {
  if (path.empty()) {
    return false;
  }
  std::string::size_type pos = 0;
  while ((pos = path.find_first_of(path_separator, pos + 1)) !=
         std::string::npos) {
    std::string sub_path = path.substr(0, pos);
    if (sub_path.empty()) {
      continue;
    }
#ifdef _WIN32
    if (_mkdir(sub_path.c_str()) != 0 && errno != EEXIST)
#else
    if (mkdir(sub_path.c_str(), mode) != 0 && errno != EEXIST)
#endif
    {
      return false;
    }
  }
#ifdef _WIN32
  if (_mkdir(path.c_str()) != 0 && errno != EEXIST)
#else
  if (mkdir(path.c_str(), mode) != 0 && errno != EEXIST)
#endif
    return false;
  return true;
}

/**
 * @brief Check whether a file or directory exists.
 *
 * @param path The file or directory path to check.
 * @return bool True if the file or directory exists, false otherwise.
 */
inline bool path_exists(const std::string &path) {
#ifdef _WIN32
  DWORD attrs = GetFileAttributesA(path.c_str());
  if (attrs == INVALID_FILE_ATTRIBUTES) {
    return false;
  }
  return true;
#else
  struct stat st;
  if (stat(path.c_str(), &st) == 0) {
    return true;
  }
  if (errno == ENOENT) {
    return false;
  }
  return false;
#endif
}

/**
 * @brief Check whether a path is a directory.
 *
 * @param path The path to check.
 * @return bool True if the path is a directory, false otherwise.
 */
inline bool is_dir(const std::string &path) {
#ifdef _WIN32
  DWORD attrs = GetFileAttributesA(path.c_str());
  if (attrs == INVALID_FILE_ATTRIBUTES) {
    return false;
  }
  return (attrs & FILE_ATTRIBUTE_DIRECTORY);
#else
  struct stat st;
  if (stat(path.c_str(), &st) == 0) {
    return S_ISDIR(st.st_mode);
  }
  return false;
#endif
}

/**
 * @brief 检查指定路径是否是文件
 *
 * @param path 要检查的路径
 * @return true 如果指定路径是文件
 * @return false 如果指定路径不存在、不是文件或者出错
 */
inline bool is_file(const std::string &path) {
  struct stat statbuf;
  if (stat(path.c_str(), &statbuf) == -1) {
    return false;
  }
  return S_ISREG(statbuf.st_mode);
}

/**
 * @brief List all files and directories in a directory.
 *
 * @param path The path to the directory.
 * @param entries A vector to store the entries in the directory.
 * @return true if the directory is listed successfully, false otherwise.
 */
inline bool list_dir(const std::string &path,
                     std::vector<std::string> &entries) {
  entries.clear();
#ifdef _WIN32
  std::string pattern = path + "\\*";
  WIN32_FIND_DATAA find_data;
  HANDLE handle = FindFirstFileA(pattern.c_str(), &find_data);
  if (handle == INVALID_HANDLE_VALUE) {
    return false;
  }
  do {
    std::string entry_name = find_data.cFileName;
    if (entry_name != "." && entry_name != "..") {
      entries.push_back(entry_name);
    }
  } while (FindNextFileA(handle, &find_data) != 0);
  FindClose(handle);
#else
  DIR *dir = opendir(path.c_str());
  if (!dir) {
    return false;
  }
  struct dirent *entry;
  while ((entry = readdir(dir)) != nullptr) {
    std::string entry_name = entry->d_name;
    if (entry_name != "." && entry_name != "..") {
      entries.push_back(entry_name);
    }
  }
  closedir(dir);
#endif
  return true;
}

/**
 * @brief Remove the file or directory at the given path.
 *
 * @param path The path to remove.
 * @return true if the operation was successful, false otherwise.
 */
inline bool remove_path(const std::string &path) {
  if (is_dir(path)) {
    // directory
    std::vector<std::string> entries;
    if (!list_dir(path, entries)) {
      return false;
    }
    for (const auto &entry : entries) {
      if (entry != "." && entry != "..") {
        const std::string full_path = path_join(path, entry);
        if (!remove_path(full_path)) {
          return false;
        }
      }
    }
#ifdef _WIN32
    if (_rmdir(path.c_str()) != 0) {
#else
    if (rmdir(path.c_str()) != 0) {
#endif
      return false;
    }
  } else {
    // file
#ifdef _WIN32
    if (_unlink(path.c_str()) != 0) {
#else
    if (unlink(path.c_str()) != 0) {
#endif
      return false;
    }
  }
  return true;
}

/**
 * @brief Normalize a pathname by collapsing redundant separators and up-level
 * references.
 *
 * @param path Path to be normalized.
 * @return Normalized path.
 */
inline std::string normpath(const std::string &path) {
  if (path.empty() || path == ".") {
    return ".";
  }

  std::vector<std::string> parts;
  std::stringstream ss(path);
  std::string part;

  // Split the path into its components
  while (std::getline(ss, part, path_separator)) {
    if (part == "" || part == ".") {
      continue;
    }
    if (part == ".." && !parts.empty() && parts.back() != "..") {
      parts.pop_back();
    } else {
      parts.push_back(part);
    }
  }

  // Recombine the parts into a path
  std::string normalized_path;
  for (const auto &part : parts) {
    normalized_path += part + path_separator;
  }

  // Remove the trailing separator, unless the path is just "/"
  if (normalized_path.size() > 1 && normalized_path.back() == path_separator) {
    normalized_path.pop_back();
  }

  // Add the root separator if the path had one
  if (path.size() > 0 && path[0] == path_separator) {
    normalized_path = path_separator + normalized_path;
  }

  return normalized_path;
}

/**
 * @brief Split a path into its directory and file components.
 *
 * This function takes a path string and returns a pair of strings: the
 * directory component and the file component. The directory component is the
 * portion of the path up to (but not including) the final path separator. The
 * file component is the portion of the path after the final path separator.
 *
 * On Windows, both forward slashes ('/') and backslashes ('\\') are recognized
 * as path separators.
 *
 * If the path does not contain a path separator, the directory component is the
 * empty string and the file component is the entire path.
 *
 * @param path The path to split.
 * @return A pair of strings: the directory component and the file component.
 */
inline std::pair<std::string, std::string> path_split(const std::string &path) {
  size_t pos = path.find_last_of(path_separator);
  if (pos == std::string::npos) {
    return {"", path};
  } else {
    return {path.substr(0, pos), path.substr(pos + 1)};
  }
}

/**
 * @brief Split a pathname into a pair (root, ext).
 *
 * @param path Pathname to split.
 * @return A pair (root, ext). ext is empty if path has no
 *         extension. If path ends with a dot, it is considered the
 *         extension. The root component is everything that comes
 *         before the last dot in the path.
 */
inline std::pair<std::string, std::string> splitext(const std::string &path) {
  // Find the position of the last dot in the path
  auto last_dot = path.find_last_of('.');
  if (last_dot == std::string::npos) {
    // No dot found, return the whole path as root with empty extension
    return std::make_pair(path, "");
  } else if (last_dot == 0) {
    // Dot at the beginning, return the whole path as extension with empty root
    return std::make_pair(path, "");
  } else {
    // Split the path at the last dot position
    auto root = path.substr(0, last_dot);
    auto ext = path.substr(last_dot);
    return std::make_pair(root, ext);
  }
}

/// 获取文件扩展名（不带.）
inline std::string get_file_ext(const std::string &fpath) {
  std::string ext = splitext(fpath).second;
  return ext.empty() ? ext : ext.substr(1);
}

/// 删除扩展名
inline std::string remove_file_ext(const std::string &filename) {
  std::string filename_without_ext = splitext(filename).first;
  return filename_without_ext;
}

/**
 * @brief 返回路径的目录部分（最后一个分隔符之前的部分）
 *
 * @param path 要处理的路径字符串
 * @return std::string 返回目录部分的字符串
 */
inline std::string dirname(const std::string &path) {
  // 如果路径为空，则返回当前目录 "."
  if (path.empty()) {
    return ".";
  }

  // 如果路径只有一个字符，则返回当前目录 "."
  if (path.size() == 1) {
    return (path[0] == path_separator) ? std::string(1, path_separator) : ".";
  }

  // 找到最后一个分隔符的位置
  auto pos = path.find_last_of(path_separator);

  // 如果没有分隔符，则返回当前目录 "."
  if (pos == std::string::npos) {
    return ".";
  }

  // 如果分隔符在第一个位置，则返回 "/"
  if (pos == 0) {
    return std::string(1, path_separator);
  }

  // 返回分隔符之前的部分
  return path.substr(0, pos);
}

/**
 * @brief 返回路径的文件名部分（不含目录部分）
 *
 * @param path 要处理的路径字符串
 * @return std::string 返回文件名部分的字符串
 */
inline std::string basename(const std::string &path) {
  // 如果路径为空，则返回空字符串
  if (path.empty()) {
    return "";
  }

  // 如果路径只有一个字符，则返回该字符
  if (path.size() == 1) {
    return (path[0] == path_separator) ? std::string(1, path_separator)
                                       : std::string(1, path[0]);
  }

  // 找到最后一个分隔符的位置
  auto pos = path.find_last_of(path_separator);

  // 如果没有分隔符，则返回原路径
  if (pos == std::string::npos) {
    return path;
  }

  // 返回分隔符之后的部分
  return path.substr(pos + 1);
}

/**
 * Lists the contents of a directory specified by the given path, filtered by
 * the provided filter function.
 *
 * @param path The path of the directory to list.
 * @param filter The filter function to apply to the directory contents. Only
 * files that pass the filter will be included in the result.
 * @return A vector of strings representing the names of the files in the
 * directory that passed the filter.
 */
inline std::vector<std::string> list_dir(
    const std::string &path,
    const std::function<bool(const std::string &)> &filter) {
  std::vector<std::string> filenames;

  struct dirent **namelist;
  int n = scandir(path.c_str(), &namelist, nullptr, versionsort);
  if (n >= 0) {
    for (int i = 0; i < n; ++i) {
      auto f = namelist[i];
      if (f->d_type == DT_REG && filter(f->d_name)) {
        filenames.emplace_back(f->d_name);
      }
      free(f);
    }
    free(namelist);
  }

  return filenames;
}

/**
 * Lists all files in the specified directory that have the specified
 * extensions.
 *
 * @param path The path to the directory to list files from.
 * @param exts A vector of file extensions to filter by. If empty, all files
 * will be returned.
 * @return A vector of file paths in the specified directory that have the
 * specified extensions.
 */
inline std::vector<std::string> list_dir(
    const std::string &path, const std::vector<std::string> &exts = {}) {
  std::vector<std::string> filenames;

  auto filter = [&](const std::string &filename) {
    if (exts.empty()) {
      return true;
    }
    std::string ext = get_file_ext(filename);
    return std::find(exts.begin(), exts.end(), ext) != exts.end();
  };

  filenames = list_dir(path, filter);

  return filenames;
}

/**
 * @brief Recursively walks a directory tree and returns information about each
 * directory and file encountered
 *
 * The function walks a directory tree, and for each directory and file it
 * encounters, returns information about the directory or file in a tuple. The
 * returned vector contains a tuple for each directory visited, and each tuple
 * contains three elements:
 *
 * 1. The absolute path of the directory
 * 2. A vector of the names of the subdirectories in the directory
 * 3. A vector of the names of the files in the directory
 *
 * The function accepts a callback function, which will be called for each
 * directory visited. The callback function can be used to customize the
 * behavior of the directory traversal.
 *
 * @param path The path to the directory to start the traversal from
 * @return A vector containing a tuple for each directory visited. Each tuple
 * contains three elements: the absolute path of the directory, a vector of the
 * names of the subdirectories in the directory, and a vector of the names of
 * the files in the directory.
 */
inline std::vector<
    std::tuple<std::string, std::vector<std::string>, std::vector<std::string>>>
path_walk(const std::string &root_path) {
  std::vector<std::tuple<std::string, std::vector<std::string>,
                         std::vector<std::string>>>
      result;

  std::vector<std::string> subdir_list;
  std::vector<std::string> file_list;
  subdir_list.push_back(root_path);

  for (size_t i = 0; i < subdir_list.size(); i++) {
    std::vector<std::string> dirnames;
    std::vector<std::string> filenames;

    std::string subdir = subdir_list[i];

    // Windows 未验证
#ifdef _WIN32
    std::string pattern = subdir + path_separator + "*.*";
    WIN32_FIND_DATAA data;
    HANDLE hFind;
    if ((hFind = FindFirstFileA(pattern.c_str(), &data)) !=
        INVALID_HANDLE_VALUE) {
      do {
        std::string filename = data.cFileName;
        if (filename == "." || filename == "..") continue;

        std::string filepath = subdir + path_separator + filename;
        if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
          subdir_list.push_back(filepath);
        } else {
          file_list.push_back(filepath);
        }
      } while (FindNextFileA(hFind, &data) != 0);

      FindClose(hFind);
    }
#else
    DIR *dirp = opendir(subdir.c_str());
    if (dirp == NULL) {
      continue;
    }

    struct dirent *dp;
    struct stat statbuf;
    while ((dp = readdir(dirp)) != NULL) {
      if (std::string(dp->d_name) == "." || std::string(dp->d_name) == "..") {
        continue;
      }

      std::string filepath = subdir + path_separator + std::string(dp->d_name);
      if (lstat(filepath.c_str(), &statbuf) == -1) {
        continue;
      }

      if (S_ISDIR(statbuf.st_mode)) {
        subdir_list.push_back(filepath);
        dirnames.emplace_back(dp->d_name);
      } else {
        file_list.push_back(filepath);
        filenames.emplace_back(dp->d_name);
      }
    }

    closedir(dirp);
#endif

    result.push_back(std::make_tuple(subdir, dirnames, filenames));
  }

  return result;
}

/**
 * @brief Recursively walk through a directory and execute the callback function
 * for each file encountered.
 *
 * @param path The path to the directory to walk through.
 * @param cb The callback function to execute for each file encountered. The
 * absolute path of the file is passed as the argument.
 */
inline void walkdir(const std::string &path,
                    const std::function<void(const std::string &)> &cb) {
#ifdef _WIN32
  std::string pattern = path + path_separator + "*";
  WIN32_FIND_DATA data;
  HANDLE handle = FindFirstFile(pattern.c_str(), &data);

  if (handle != INVALID_HANDLE_VALUE) {
    do {
      std::string filename = data.cFileName;
      if (filename != "." && filename != "..") {
        std::string filepath = path + path_separator + filename;
        if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
          walkdir(filepath, cb);
        } else {
          cb(filepath);
        }
      }
    } while (FindNextFile(handle, &data));
    FindClose(handle);
  }
#else
  DIR *dir = opendir(path.c_str());
  if (dir) {
    struct dirent *entry;
    while ((entry = readdir(dir))) {
      std::string filename = entry->d_name;
      if (filename != "." && filename != "..") {
        std::string filepath = path + path_separator + filename;
        if (entry->d_type == DT_DIR) {
          walkdir(filepath, cb);
        } else {
          cb(filepath);
        }
      }
    }
    closedir(dir);
  }
#endif
}

/**
 * @brief 获取文件大小
 *
 * @param path 文件路径
 * @return std::size_t 文件大小（单位：字节）
 */
inline std::size_t getsize(const std::string &path) {
  std::size_t size = 0;

#ifdef _WIN32
  // Windows 平台下使用 GetFileSizeEx 获取文件大小
  HANDLE hFile =
      CreateFile(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr,
                 OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
  if (hFile != INVALID_HANDLE_VALUE) {
    LARGE_INTEGER fileSize;
    if (GetFileSizeEx(hFile, &fileSize)) {
      size = static_cast<std::size_t>(fileSize.QuadPart);
    }
    CloseHandle(hFile);
  }
#else
  // Unix 平台下使用 stat 获取文件大小
  struct stat st;
  if (stat(path.c_str(), &st) == 0) {
    size = static_cast<std::size_t>(st.st_size);
  }
#endif

  return size;
}

/// 确保文件父目录存在
inline std::string valid_filepath(std::string const &fpath) {
  makedirs(dirname(fpath));
  return fpath;
}

inline std::string get_now_datetime_path() {
  auto t = std::time(nullptr);
  auto tm = *std::localtime(&t);

  std::ostringstream oss;
  std::string fmt = std::string("%Y-%m-%d") + path_separator + "%H-%M-%S";
  oss << std::put_time(&tm, fmt.c_str());

  return oss.str();
}

/**
 * @brief Determine if a file is a image file by its extension.
 *
 * @param path The path to the file.
 * @return true if the file is a image file, false otherwise.
 */
inline bool is_image(const std::string &path) {
  static const std::set<std::string> image_exts = {
      "bmp", "gif", "jpeg", "jpg", "png", "svg", "tif", "tiff", "webp"};

  // Get the file extension
  std::string ext = get_file_ext(path);

  // Check if the file extension is an image format
  return image_exts.count(ext) > 0;
}

/**
 * @brief Determine if a file is a video file by its extension.
 *
 * @param path The path to the file.
 * @return true if the file is a video file, false otherwise.
 */
inline bool is_video(const std::string &path) {
  static const std::set<std::string> video_exts = {
      "avi", "mp4", "mkv", "mov", "wmv", "flv", "f4v",  "rmvb", "rm",
      "3gp", "dat", "ts",  "mts", "vob", "mpg", "mpeg", "m4v",  "webm"};

  // Get the file extension
  std::string ext = get_file_ext(path);

  // Check if the file extension is a video format
  return video_exts.count(ext) > 0;
}

///  判断是否为在线视频
inline bool is_online_video(const std::string &url) {
  static const std::vector<std::string> remote_protocols = {
      "http://", "https://", "ftp://", "sftp://", "rtsp://", "rtmp://"};
  return std::any_of(
      remote_protocols.begin(), remote_protocols.end(),
      [&](const std::string &protocol) { return url.find(protocol) == 0; });
}

/**
 * @brief Lists all image files in the specified directory.
 *
 * This function returns a vector of strings containing the names of all image
 * files (i.e., files with extensions .jpg, .jpeg, .png, .bmp, .gif) in the
 * specified directory.
 *
 * @param path The path to the directory to search for image files. Defaults to
 * the current directory.
 *
 * @return A vector of strings containing the names of all image files in the
 * specified directory.
 */
inline std::vector<std::string> list_images(const std::string &path = ".") {
  return list_dir(path, is_image);
}

/**
 * @brief Lists all video files in the specified directory.
 *
 * This function returns a vector of strings containing the names of all video
 * files (i.e., files with extensions .avi, .mp4, .mkv, .mov, .wmv, .flv, .f4v,
 * .rmvb, .rm, .3gp, .dat, .ts, .mts, .vob, .mpg, .mpeg, .m4v, .webm) in the
 * specified directory.
 *
 * @param path The path to the directory to search for video files. Defaults to
 * the current directory.
 *
 * @return A vector of strings containing the names of all video files in the
 * specified directory.
 */
inline std::vector<std::string> list_videos(const std::string &path = ".") {
  return list_dir(path, is_video);
}

// ==============================================================================================
//                                         文件读写
// ==============================================================================================

/// 读取整个文件内容到string
inline void read_file_to_string(const std::string &infile,
                                std::string &outstr) {
  std::ifstream in(infile, std::ios::in);
  if (!in.is_open()) {
    std::cerr << "open file " << infile << " failed!" << std::endl;
    return;
  }
  std::istreambuf_iterator<char> beg(in), end;
  outstr.assign(beg, end);
  in.close();
}

/// 读取整个文件内容 (说明：由于RVO的存在，此函数性能与 read_file_to_string
/// 差不多，而且更简洁，因此更推荐)
inline std::string read_file(const std::string &infile) {
  std::string s;
  read_file_to_string(infile, s);
  return s;  // RVO
}

}  // namespace
