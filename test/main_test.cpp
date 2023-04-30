#include "gtest/gtest.h"

#include "fs_utils.h"

TEST(fs_utils, path_join) {
// 非 Windows 系统下的测试用例
#ifndef _WIN32
  EXPECT_EQ(path_join("foo", "bar"), "foo/bar");
  EXPECT_EQ(path_join("foo/", "bar"), "foo/bar");
  EXPECT_EQ(path_join("foo", "/bar"), "foo/bar");
  EXPECT_EQ(path_join("/foo", "bar"), "/foo/bar");
  EXPECT_EQ(path_join("/foo/", "bar"), "/foo/bar");
  EXPECT_EQ(path_join("/foo", "/bar"), "/foo/bar");
  EXPECT_EQ(path_join("foo", "bar", "baz"), "foo/bar/baz");
  EXPECT_EQ(path_join("/foo", "bar", "baz"), "/foo/bar/baz");
  EXPECT_EQ(path_join("/foo", "/bar/", "baz/"), "/foo/bar/baz/");
  EXPECT_EQ(path_join("/foo", "/bar", "/baz"), "/foo/bar/baz");
  EXPECT_EQ(path_join("foo"), "foo");
  EXPECT_EQ(path_join("/foo"), "/foo");
  EXPECT_EQ(path_join(""), "");
  EXPECT_EQ(path_join("/"), "/");
  EXPECT_EQ(path_join("/", "foo"), "/foo");
  EXPECT_EQ(path_join("foo", ""), "foo/");
  EXPECT_EQ(path_join("", ""), "");
#endif

// Windows 系统下的测试用例 (未测试)
#ifdef _WIN32
  EXPECT_EQ(path_join("C:\\", "Program Files"), "C:\\Program Files");
  EXPECT_EQ(path_join("\\\\server\\share", "dir"), "\\\\server\\share\\dir");
  EXPECT_EQ(path_join("\\\\server\\share\\", "dir"), "\\\\server\\share\\dir");
  EXPECT_EQ(path_join("\\\\server\\share", "\\dir"), "\\\\server\\share\\dir");
  EXPECT_EQ(path_join("C:\\", "Program Files", "Microsoft Visual Studio"),
            "C:\\Program Files\\Microsoft Visual Studio");
  EXPECT_EQ(path_join("\\\\server\\share", "dir1", "dir2"),
            "\\\\server\\share\\dir1\\dir2");
  EXPECT_EQ(path_join("\\\\server\\share\\", "dir1", "dir2"),
            "\\\\server\\share\\dir1\\dir2");
  EXPECT_EQ(path_join("\\\\server\\share", "\\dir1", "\\dir2"),
            "\\\\server\\share\\dir1\\dir2");
  EXPECT_EQ(path_join("C:\\", "", "Program Files", "Microsoft Visual Studio"),
            "C:\\Program Files\\Microsoft Visual Studio");
  EXPECT_EQ(path_join(""), "");
  EXPECT_EQ(path_join("\\", ""), "\\");
#endif
}

TEST(fs_utils, SingleDirTest) {
  // create a single directory
  const std::string dir_name = "test_dir";
  ASSERT_FALSE(path_exists(dir_name));
  ASSERT_TRUE(makedirs(dir_name));
  ASSERT_TRUE(path_exists(dir_name));
  ASSERT_TRUE(is_dir(dir_name));
  remove_path(dir_name);
  ASSERT_FALSE(path_exists(dir_name));
}

TEST(fs_utils, MultiLevelDirTest) {
#ifdef _WIN32
  // create multiple level directories
  const std::string dir_name = "test_dir\\test_subdir\\test_subsubdir";
#else
  const std::string dir_name = "test_dir/test_subdir/test_subsubdir";
#endif
  ASSERT_FALSE(path_exists(dir_name));
  ASSERT_TRUE(makedirs(dir_name));
  ASSERT_TRUE(path_exists(dir_name));
  ASSERT_TRUE(is_dir(dir_name));
  remove_path(dir_name);
  ASSERT_FALSE(path_exists(dir_name));
}

TEST(fs_utils, is_file) {
  std::string existing_file = __FILE__;
  EXPECT_TRUE(is_file(existing_file));

  std::string non_existing_file = "non_existing_file.txt";
  EXPECT_FALSE(is_file(non_existing_file));

  std::string directory_path = ".";
  EXPECT_FALSE(is_file(directory_path));

#ifndef _WIN32
  std::string symbolic_link =
      "/bin/sh"; // This is a symbolic link in most Unix-like systems
  EXPECT_TRUE(is_file(symbolic_link));
#endif
}

TEST(fs_utils, list_dir) {
  // Create a temporary directory
  std::string dir_path = "test_dir";
  makedirs(dir_path);

  // Create some files in the directory
  std::vector<std::string> expected_files = {"file1.txt", "file2.txt",
                                             "file3.txt"};
  for (const auto &file : expected_files) {
    std::ofstream outfile(dir_path + path_separator + file);
    outfile << "test file content" << std::endl;
  }

  // Call list_dir to get the list of files
  std::vector<std::string> actual_files;
  ASSERT_TRUE(list_dir(dir_path, actual_files));

  // Check if the actual list matches the expected list
  ASSERT_EQ(expected_files.size(), actual_files.size());
  for (const auto &file : expected_files) {
    ASSERT_NE(std::find(actual_files.begin(), actual_files.end(), file),
              actual_files.end());
  }

  // Remove the temporary directory and its contents
  remove_path(dir_path);
}

TEST(fs_utils, normpath) {
#ifndef _WIN32
  // Test empty path
  EXPECT_EQ(normpath(""), ".");

  // Test path with just a single dot
  EXPECT_EQ(normpath("."), ".");

  // Test path with double dots
  EXPECT_EQ(normpath(".."), "..");
  EXPECT_EQ(normpath("../.."), "../..");

  // Test path with double dots and trailing slash
  EXPECT_EQ(normpath("../"), "..");
  EXPECT_EQ(normpath("../../"), "../..");

  // Test path with multiple consecutive slashes
  EXPECT_EQ(normpath("foo//bar"), "foo/bar");

  // Test path with single dots
  EXPECT_EQ(normpath("./foo/./bar/."), "foo/bar");

  // Test path with mixed dots and double dots
  EXPECT_EQ(normpath("./foo/../bar/./baz/../qux"), "bar/qux");

  // Test path with backslashes on Windows
#else
  EXPECT_EQ(normpath("foo\\bar\\baz"), "foo\\bar\\baz");
  EXPECT_EQ(normpath("foo\\\\bar\\\\baz"), "foo\\bar\\baz");
  EXPECT_EQ(normpath("foo\\bar\\\\baz"), "foo\\bar\\baz");
  EXPECT_EQ(normpath("foo\\..\\bar"), "bar");
  EXPECT_EQ(normpath("foo\\.\\bar\\.\\baz"), "foo\\bar\\baz");
  EXPECT_EQ(normpath("foo\\.\\bar\\..\\baz\\qux"), "foo\\baz\\qux");
#endif
}

TEST(fs_utils, path_split) { // Test case 1
  std::string path = "foo/bar/baz.txt";
  std::string dir, file;
  std::tie(dir, file) = path_split(path);
  EXPECT_EQ(dir, "foo/bar");
  EXPECT_EQ(file, "baz.txt");

  // Test case 2
  path = "foo";
  std::tie(dir, file) = path_split(path);
  EXPECT_EQ(dir, "");
  EXPECT_EQ(file, "foo");

  // Test case 3
  path = "/foo/bar/baz.txt";
  std::tie(dir, file) = path_split(path);
  EXPECT_EQ(dir, "/foo/bar");
  EXPECT_EQ(file, "baz.txt");

  // Test case 4
  path = "/foo/bar/";
  std::tie(dir, file) = path_split(path);
  EXPECT_EQ(dir, "/foo/bar");
  EXPECT_EQ(file, "");
}

TEST(fs_utils, splitext) { // Test case 1
  std::string path = "foo/bar/baz.txt";
  std::string root, ext;
  std::tie(root, ext) = splitext(path);
  EXPECT_EQ(root, "foo/bar/baz");
  EXPECT_EQ(ext, ".txt");

  // Test case 2
  path = "foo/bar/baz.";
  std::tie(root, ext) = splitext(path);
  EXPECT_EQ(root, "foo/bar/baz");
  EXPECT_EQ(ext, ".");

  // Test case 3
  path = "foo/bar/baz";
  std::tie(root, ext) = splitext(path);
  EXPECT_EQ(root, "foo/bar/baz");
  EXPECT_EQ(ext, "");

  // Test case 4
  path = ".bashrc";
  std::tie(root, ext) = splitext(path);
  EXPECT_EQ(root, ".bashrc");
  EXPECT_EQ(ext, "");

  // Test case 5
  path = "";
  std::tie(root, ext) = splitext(path);
  EXPECT_EQ(root, "");
  EXPECT_EQ(ext, "");
}

TEST(fs_utils, dirname) {
#ifndef _WIN32
  EXPECT_EQ(dirname("/usr/local/bin"), "/usr/local");
  EXPECT_EQ(dirname("/usr/local/"), "/usr/local");
  EXPECT_EQ(dirname("/usr/local"), "/usr");
  EXPECT_EQ(dirname("/usr/"), "/usr");
  EXPECT_EQ(dirname("/usr"), "/");
  EXPECT_EQ(dirname("/"), "/");
  EXPECT_EQ(dirname(""), ".");
  EXPECT_EQ(dirname("file.txt"), ".");
  EXPECT_EQ(dirname("/file.txt"), "/");
  EXPECT_EQ(dirname("path/to/file.txt"), "path/to");
  EXPECT_EQ(dirname("/path/to/file.txt"), "/path/to");
#else
  EXPECT_EQ(dirname("C:\\Windows\\System32"), "C:\\Windows");
  EXPECT_EQ(dirname("C:\\Windows\\"), "C:\\Windows");
  EXPECT_EQ(dirname("C:\\Windows"), "C:\\");
  EXPECT_EQ(dirname("C:\\Program Files\\"), "C:\\");
  EXPECT_EQ(dirname("C:\\Windows\\System32\\cmd.exe"), "C:\\Windows\\System32");
  EXPECT_EQ(dirname("cmd.exe"), ".");
  EXPECT_EQ(dirname(""), ".");
  EXPECT_EQ(dirname("file.txt"), ".");
  EXPECT_EQ(dirname("C:\\file.txt"), "C:\\");
  EXPECT_EQ(dirname("C:\\path\\to\\file.txt"), "C:\\path\\to");
#endif
}

TEST(fs_utils, basename) {
  basename("");
#ifndef _WIN32
  EXPECT_EQ(basename(std::string("/usr/local/bin")), "bin");
  EXPECT_EQ(basename(std::string("/usr/local/")), "");
  EXPECT_EQ(basename(std::string("/usr/local")), "local");
  EXPECT_EQ(basename(std::string("/usr/")), "");
  EXPECT_EQ(basename(std::string("/")), "/");
  EXPECT_EQ(basename(std::string("")), "");
  EXPECT_EQ(basename(std::string("file.txt")), "file.txt");
  EXPECT_EQ(basename(std::string("/file.txt")), "file.txt");
  EXPECT_EQ(basename(std::string("path/to/file.txt")), "file.txt");
  EXPECT_EQ(basename(std::string("/path/to/file.txt")), "file.txt");
#else
  EXPECT_EQ(basename("C:\\Windows\\System32"), "System32");
  EXPECT_EQ(basename("C:\\Windows\\"), "Windows");
  EXPECT_EQ(basename("C:\\Windows"), "Windows");
  EXPECT_EQ(basename("C:\\Program Files\\"), "Program Files");
  EXPECT_EQ(basename("C:\\Windows\\System32\\cmd.exe"), "cmd.exe");
  EXPECT_EQ(basename("cmd.exe"), "cmd.exe");
  EXPECT_EQ(basename(""), ".");
  EXPECT_EQ(basename("file.txt"), "file.txt");
  EXPECT_EQ(basename("C:\\file.txt"), "file.txt");
  EXPECT_EQ(basename("C:\\path\\to\\file.txt"), "file.txt");
#endif
}

TEST(fs_utils, get_file_ext) {
  EXPECT_EQ(get_file_ext("file.txt"), "txt");
  EXPECT_EQ(get_file_ext("test.tar.gz"), "gz");
  EXPECT_EQ(get_file_ext(".hidden"), "");
  EXPECT_EQ(get_file_ext("no_extension"), "");
  EXPECT_EQ(get_file_ext("doc.docx"), "docx");
}

TEST(fs_utils, remove_file_ext) {
  EXPECT_EQ(remove_file_ext("file.txt"), "file");
  EXPECT_EQ(remove_file_ext("test.tar.gz"), "test.tar");
  EXPECT_EQ(remove_file_ext(".hidden"), ".hidden");
  EXPECT_EQ(remove_file_ext("no_extension"), "no_extension");
  EXPECT_EQ(remove_file_ext("doc.docx"), "doc");
}

TEST(fs_utils, path_walk) {
  std::vector<std::tuple<std::string, std::vector<std::string>,
                         std::vector<std::string>>> const expected{
      {"test_dir", {"sub3", "sub1"}, {"file.txt"}},
      {"test_dir/sub3", {}, {"file4.txt"}},
      {"test_dir/sub1", {"sub2"}, {"file2.txt"}},
      {"test_dir/sub1/sub2", {}, {"file3.txt"}},
  };
  // 构建测试目录
  for (auto const &iter : expected) {
    auto const &dir = std::get<0>(iter);
    auto const &subdirs = std::get<1>(iter);
    auto const &files = std::get<2>(iter);
    makedirs(dir);
    for (auto const &subdir : subdirs) {
      makedirs(path_join(dir, subdir));
    }
    for (auto const &file : files) {
      std::ofstream(path_join(dir, file));
    }
  }

  auto result = path_walk("test_dir");

  EXPECT_EQ(result, expected);

  walkdir("test_dir",
          [](std::string const &path) { std::cout << path << std::endl; });

  // 清理测试目录
  remove_path("test_dir");
}

TEST(fs_utils, get_now_datetime_path) {
  std::cout << get_now_datetime_path() << std::endl;
}

// TEST(fs_utils, fs_utils)
//{
//     std::cout << listdir() << std::endl;
//     auto images = list_images("results");
//     // std::sort(images.begin(), images.end(), compareNat);
//     std::cout << images << std::endl;
//     std::cout << is_file(".clang-format") << std::endl;
//     std::cout << is_file(".clang-format1") << std::endl;
//     std::cout << is_file("src") << std::endl;
//     std::cout << is_directory(".clang-format") << std::endl;
//     std::cout << is_directory("src") << std::endl;
//     std::cout << is_directory("src1") << std::endl;
//     auto cb = [](const std::string &path) { std::cout << path << std::endl;
//     }; walkdir(".", cb);
// }

TEST(fs_utils, is_image) {
  EXPECT_TRUE(is_image("/path/to/test.jpg"));
  EXPECT_TRUE(is_image("/path/to/test.jpeg"));
  EXPECT_TRUE(is_image("/path/to/test.png"));
  EXPECT_TRUE(is_image("/path/to/test.bmp"));
  EXPECT_TRUE(is_image("/path/to/test.gif"));
  EXPECT_TRUE(is_image("/path/to/test.webp"));
  EXPECT_TRUE(is_image("/path/to/test.tiff"));
  EXPECT_TRUE(is_image("/path/to/test.svg"));

  EXPECT_FALSE(is_image("/path/to/test.txt"));
  EXPECT_FALSE(is_image("/path/to/test.pdf"));
  EXPECT_FALSE(is_image("/path/to/test"));
  EXPECT_FALSE(is_image(""));
}

TEST(fs_utils, is_video) {
  EXPECT_TRUE(is_video("/path/to/test.mp4"));
  EXPECT_TRUE(is_video("/path/to/test.mkv"));
  EXPECT_TRUE(is_video("/path/to/test.avi"));
  EXPECT_TRUE(is_video("/path/to/test.wmv"));
  EXPECT_TRUE(is_video("/path/to/test.mov"));
  EXPECT_TRUE(is_video("/path/to/test.flv"));
  EXPECT_TRUE(is_video("/path/to/test.webm"));
  EXPECT_TRUE(is_video("/path/to/test.m4v"));

  EXPECT_FALSE(is_video("/path/to/test.txt"));
  EXPECT_FALSE(is_video("/path/to/test.pdf"));
  EXPECT_FALSE(is_video("/path/to/test"));
  EXPECT_FALSE(is_video(""));
}

TEST(fs_utils, is_online_video) {
  EXPECT_TRUE(is_online_video("http://example.com/video.mp4"));
  EXPECT_TRUE(is_online_video("https://example.com/video.mp4"));
  EXPECT_TRUE(is_online_video("ftp://example.com/video.mp4"));
  EXPECT_TRUE(is_online_video("sftp://example.com/video.mp4"));
  EXPECT_TRUE(is_online_video("rtsp://example.com/video.mp4"));

  EXPECT_FALSE(is_online_video("example.com/video.mp4"));
  EXPECT_FALSE(is_online_video("file:///home/user/video.mp4"));
  EXPECT_FALSE(is_online_video(""));
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
