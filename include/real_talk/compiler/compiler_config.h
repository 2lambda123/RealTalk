
#ifndef _REAL_TALK_COMPILER_COMPILER_CONFIG_H_
#define _REAL_TALK_COMPILER_COMPILER_CONFIG_H_

#include <boost/filesystem.hpp>
#include <vector>

namespace real_talk {
namespace compiler {

class CompilerConfig {
 public:
  explicit CompilerConfig(const boost::filesystem::path &input_file_path);
  void SetSrcDirPath(const boost::filesystem::path &path);
  void SetBinDirPath(const boost::filesystem::path &path);
  void SetVendorDirPath(const boost::filesystem::path &path);
  void SetImportDirPaths(const std::vector<boost::filesystem::path> &paths);
  const boost::filesystem::path &GetInputFilePath() const;
  const boost::filesystem::path &GetSrcDirPath() const;
  const boost::filesystem::path &GetBinDirPath() const;
  const boost::filesystem::path &GetVendorDirPath() const;
  const std::vector<boost::filesystem::path> &GetImportDirPaths() const;
};
}
}
#endif
