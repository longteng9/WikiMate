#ifndef ZIPARCHIVE_H
#define ZIPARCHIVE_H
#include <string>
#include <vector>
#include <map>
#define CASESENSITIVITY (0)
#define WRITEBUFFERSIZE (16384)
#define MAXFILENAME (256)

namespace zip{
    class Archive
    {
    public:
        enum class Conf{
            ZIP_OVERWRITE,
            ZIP_APPEND,
            UNZIP_OVERWRITE,
            COMPRESS_LEVEL,
            PASSWORD,
        };
    public:
        Archive();
       virtual  ~Archive();

        Archive* set(Conf conf, int val);
        Archive* set(Conf conf, const std::string& val);
        int get(Conf conf) const;
        std::string get(Conf conf, bool _=true) const;

        int compress(const std::vector<std::string>& srcList, const std::string& dst);
        int uncompress(const std::string& zipPath,
                       const std::string& dst,
                       const std::vector<std::string>& srcList = std::vector<std::string>());
        int getFilesZipped(const std::string& zipPath, std::vector<std::string> *infos);

        std::string errMsg() const;

    protected:
        int do_compress(const std::string& dst, const std::map<std::string, std::string>& srcMap);
        bool isDir(const std::string& path);
        std::string getRelativePath(const std::string& dirpath_, const std::string& filepath);
        void findAllFiles(const std::string& startPath, std::vector<std::string>* results, bool recur = true);

    protected:
        int optOverwrite = 1; //1 = overwrite existing zip file; 2 = append to existing zip file.
        int optCompressLevel;
        const int optExcludePath = 1; //enabled if you want to only store the file's name.
        int buffSize = WRITEBUFFERSIZE;
        void* buff = NULL;
        std::string password;
        std::string error;
    };
}
#endif //ZIP_ARCHIVE_H
