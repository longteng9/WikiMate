#include "Archive.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>

#ifdef _WIN32
# include <direct.h>
# include <io.h>
#else
# include <unistd.h>
# include <utime.h>
# include <sys/types.h>
# include <sys/stat.h>
#endif


#include "zip.h"
#include "unzip.h"

#ifdef _WIN32
#define USEWIN32IOAPI
#include "iowin32.h"
#endif

#if (!defined(_WIN32)) && (!defined(WIN32)) && (!defined(__APPLE__))
#ifndef __USE_FILE_OFFSET64
#define __USE_FILE_OFFSET64
#endif
#ifndef __USE_LARGEFILE64
#define __USE_LARGEFILE64
#endif
#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE
#endif
#ifndef _FILE_OFFSET_BIT
#define _FILE_OFFSET_BIT 64
#endif
#endif

#ifdef __APPLE__
// In darwin and perhaps other BSD variants off_t is a 64 bit value, hence no need for specific 64 bit functions
#define FOPEN_FUNC(filename, mode) fopen(filename, mode)
#define FTELLO_FUNC(stream) ftello(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko(stream, offset, origin)
#else
#define FOPEN_FUNC(filename, mode) fopen64(filename, mode)
#define FTELLO_FUNC(stream) ftello64(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko64(stream, offset, origin)
#endif

namespace zip{
    namespace _private_{
#ifdef _WIN32
        uLong filetime(char *f, tm_zip *tmzip, uLong *dt)
        {
            int ret = 0;
            {
                FILETIME ftLocal;
                HANDLE hFind;
                WIN32_FIND_DATAA ff32;

                hFind = FindFirstFileA(f, &ff32);
                if (hFind != INVALID_HANDLE_VALUE)
                {
                    FileTimeToLocalFileTime(&(ff32.ftLastWriteTime), &ftLocal);
                    FileTimeToDosDateTime(&ftLocal, ((LPWORD)dt) + 1, ((LPWORD)dt) + 0);
                    FindClose(hFind);
                    ret = 1;
                }
            }
            return ret;
        }
#else
#ifdef unix || __APPLE__
        uLong filetime(char *f, tm_zip *tmzip, uLong *dt)
        {
            int ret = 0;
            struct stat s;        /* results of stat() */
            struct tm* filedate;
            time_t tm_t = 0;

            if (strcmp(f, "-") != 0)
            {
                char name[MAXFILENAME + 1];
                int len = strlen(f);
                if (len > MAXFILENAME)
                    len = MAXFILENAME;

                strncpy(name, f, MAXFILENAME - 1);
                /* strncpy doesnt append the trailing NULL, of the string is too long. */
                name[MAXFILENAME] = '\0';

                if (name[len - 1] == '/')
                    name[len - 1] = '\0';
                /* not all systems allow stat'ing a file with / appended */
                if (stat(name, &s) == 0)
                {
                    tm_t = s.st_mtime;
                    ret = 1;
                }
            }
            filedate = localtime(&tm_t);

            tmzip->tm_sec = filedate->tm_sec;
            tmzip->tm_min = filedate->tm_min;
            tmzip->tm_hour = filedate->tm_hour;
            tmzip->tm_mday = filedate->tm_mday;
            tmzip->tm_mon = filedate->tm_mon;
            tmzip->tm_year = filedate->tm_year;

            return ret;
        }
#else
        uLong filetime(char *f, tm_zip *tmzip, uLong *dt)
        {
            return 0;
        }
#endif
#endif

        int check_exist_file(const char* filename)
        {
            FILE* ftestexist;
            int ret = 1;
            ftestexist = FOPEN_FUNC(filename, "rb");
            if (ftestexist == NULL)
                ret = 0;
            else
                fclose(ftestexist);
            return ret;
        }

        /* calculate the CRC32 of a file,
        because to encrypt a file, we need known the CRC32 of the file before */
        int getFileCrc(const char* filenameinzip, void*buf, unsigned long size_buf, unsigned long* result_crc)
        {
            unsigned long calculate_crc = 0;
            int err = ZIP_OK;
            FILE * fin = FOPEN_FUNC(filenameinzip, "rb");

            unsigned long size_read = 0;
            unsigned long total_read = 0;
            if (fin == NULL)
            {
                err = ZIP_ERRNO;
            }

            if (err == ZIP_OK)
            do
            {
                err = ZIP_OK;
                size_read = (int)fread(buf, 1, size_buf, fin);
                if (size_read < size_buf)
                if (feof(fin) == 0)
                {
                    printf("error in reading %s\n", filenameinzip);
                    err = ZIP_ERRNO;
                }

                if (size_read>0)
                    calculate_crc = crc32(calculate_crc, (Bytef*)buf, size_read);
                total_read += size_read;

            } while ((err == ZIP_OK) && (size_read > 0));

            if (fin)
                fclose(fin);

            *result_crc = calculate_crc;
            //printf("file %s crc %lx\n", filenameinzip, calculate_crc);
            return err;
        }

        int isLargeFile(const char* filename)
        {
            int largeFile = 0;
            ZPOS64_T pos = 0;
            FILE* pFile = FOPEN_FUNC(filename, "rb");

            if (pFile != NULL)
            {
                int n = FSEEKO_FUNC(pFile, 0, SEEK_END);
                pos = FTELLO_FUNC(pFile);

                //printf("File : %s is %lld bytes\n", filename, pos);

                if (pos >= 0xffffffff)
                    largeFile = 1;

                fclose(pFile);
            }

            return largeFile;
        }

        /* change_file_date : change the date/time of a file
            filename : the filename of the file where date/time must be modified
            dosdate : the new date at the MSDos format (4 bytes)
            tmu_date : the SAME new date at the tm_unz format */
        void change_file_date(const char *filename, uLong dosdate, tm_unz tmu_date)
        {
        #ifdef _WIN32
          HANDLE hFile;
          FILETIME ftm,ftLocal,ftCreate,ftLastAcc,ftLastWrite;

          hFile = CreateFileA(filename,GENERIC_READ | GENERIC_WRITE,
                              0,NULL,OPEN_EXISTING,0,NULL);
          GetFileTime(hFile,&ftCreate,&ftLastAcc,&ftLastWrite);
          DosDateTimeToFileTime((WORD)(dosdate>>16),(WORD)dosdate,&ftLocal);
          LocalFileTimeToFileTime(&ftLocal,&ftm);
          SetFileTime(hFile,&ftm,&ftLastAcc,&ftm);
          CloseHandle(hFile);
        #else
        #ifdef unix || __APPLE__
          struct utimbuf ut;
          struct tm newdate;
          newdate.tm_sec = tmu_date.tm_sec;
          newdate.tm_min=tmu_date.tm_min;
          newdate.tm_hour=tmu_date.tm_hour;
          newdate.tm_mday=tmu_date.tm_mday;
          newdate.tm_mon=tmu_date.tm_mon;
          if (tmu_date.tm_year > 1900)
              newdate.tm_year=tmu_date.tm_year - 1900;
          else
              newdate.tm_year=tmu_date.tm_year ;
          newdate.tm_isdst=-1;

          ut.actime=ut.modtime=mktime(&newdate);
          utime(filename,&ut);
        #endif
        #endif
        }


        /* mymkdir and change_file_date are not 100 % portable
           As I don't know well Unix, I wait feedback for the unix portion */
        int mymkdir(const char* dirname)
        {
            int ret=0;
        #ifdef _WIN32
            ret = _mkdir(dirname);
        #elif unix
            ret = mkdir (dirname,0775);
        #elif __APPLE__
            ret = mkdir (dirname,0775);
        #endif
            return ret;
        }

        int makedir (const char *newdir)
        {
          char *buffer ;
          char *p;
          int  len = (int)strlen(newdir);

          if (len <= 0)
            return 0;

          buffer = (char*)malloc(len+1);
            if (buffer==NULL)
            {
                    printf("Error allocating memory\n");
                    return UNZ_INTERNALERROR;
            }
          strcpy(buffer,newdir);

          if (buffer[len-1] == '/') {
            buffer[len-1] = '\0';
          }
          if (mymkdir(buffer) == 0)
            {
              free(buffer);
              return 1;
            }

          p = buffer+1;
          while (1)
            {
              char hold;

              while(*p && *p != '\\' && *p != '/')
                p++;
              hold = *p;
              *p = 0;
              if ((mymkdir(buffer) == -1) && (errno == ENOENT))
                {
                  printf("couldn't create directory %s\n",buffer);
                  free(buffer);
                  return 0;
                }
              if (hold == 0)
                break;
              *p++ = hold;
            }
          free(buffer);
          return 1;
        }

        int do_extract_currentfile(unzFile uf, const int* popt_extract_without_path, int* popt_overwrite, const char* password)
        {
            char filename_inzip[256];
            char* filename_withoutpath;
            char* p;
            int err=UNZ_OK;
            FILE *fout=NULL;
            void* buf;
            uInt size_buf;

            unz_file_info64 file_info;
            uLong ratio=0;
            err = unzGetCurrentFileInfo64(uf,&file_info,filename_inzip,sizeof(filename_inzip),NULL,0,NULL,0);

            if (err!=UNZ_OK)
            {
                printf("error %d with zipfile in unzGetCurrentFileInfo\n",err);
                return err;
            }

            size_buf = WRITEBUFFERSIZE;
            buf = (void*)malloc(size_buf);
            if (buf==NULL)
            {
                printf("Error allocating memory\n");
                return UNZ_INTERNALERROR;
            }

            p = filename_withoutpath = filename_inzip;
            while ((*p) != '\0')
            {
                if (((*p)=='/') || ((*p)=='\\'))
                    filename_withoutpath = p+1;
                p++;
            }

            if ((*filename_withoutpath)=='\0')
            {
                if ((*popt_extract_without_path)==0)
                {
                    printf("creating directory: %s\n",filename_inzip);
                    mymkdir(filename_inzip);
                }
            }
            else
            {
                const char* write_filename;
                int skip=0;

                if ((*popt_extract_without_path)==0)
                    write_filename = filename_inzip;
                else
                    write_filename = filename_withoutpath;

                err = unzOpenCurrentFilePassword(uf,password);
                if (err!=UNZ_OK)
                {
                    printf("error %d with zipfile in unzOpenCurrentFilePassword\n",err);
                }

                if (((*popt_overwrite)==0) && (err==UNZ_OK))
                {
                    char rep=0;
                    FILE* ftestexist;
                    ftestexist = FOPEN_FUNC(write_filename,"rb");
                    if (ftestexist!=NULL)
                    {
                        fclose(ftestexist);
                        do
                        {
                            char answer[128];
                            int ret;

                            printf("The file %s exists. Overwrite ? [y]es, [n]o, [A]ll: ",write_filename);
                            ret = scanf("%1s",answer);
                            if (ret != 1)
                            {
                               exit(EXIT_FAILURE);
                            }
                            rep = answer[0] ;
                            if ((rep>='a') && (rep<='z'))
                                rep -= 0x20;
                        }
                        while ((rep!='Y') && (rep!='N') && (rep!='A'));
                    }

                    if (rep == 'N')
                        skip = 1;

                    if (rep == 'A')
                        *popt_overwrite=1;
                }

                if ((skip==0) && (err==UNZ_OK))
                {
                    fout=FOPEN_FUNC(write_filename,"wb");
                    // some zipfile don't contain directory alone before file
                    if ((fout==NULL) && ((*popt_extract_without_path)==0) &&
                                        (filename_withoutpath!=(char*)filename_inzip))
                    {
                        char c=*(filename_withoutpath-1);
                        *(filename_withoutpath-1)='\0';
                        makedir(write_filename);
                        *(filename_withoutpath-1)=c;
                        fout=FOPEN_FUNC(write_filename,"wb");
                    }

                    if (fout==NULL)
                    {
                        printf("error opening %s\n",write_filename);
                    }
                }

                if (fout!=NULL)
                {
                    //printf(" extracting: %s\n",write_filename);

                    do
                    {
                        err = unzReadCurrentFile(uf,buf,size_buf);
                        if (err<0)
                        {
                            printf("error %d with zipfile in unzReadCurrentFile\n",err);
                            break;
                        }
                        if (err>0)
                            if (fwrite(buf,err,1,fout)!=1)
                            {
                                printf("error in writing extracted file\n");
                                err=UNZ_ERRNO;
                                break;
                            }
                    }
                    while (err>0);
                    if (fout)
                            fclose(fout);

                    if (err==0)
                        change_file_date(write_filename,file_info.dosDate,
                                         file_info.tmu_date);
                }

                if (err==UNZ_OK)
                {
                    err = unzCloseCurrentFile (uf);
                    if (err!=UNZ_OK)
                    {
                        printf("error %d with zipfile in unzCloseCurrentFile\n",err);
                    }
                }
                else
                    unzCloseCurrentFile(uf); // don't lose the error
            }

            free(buf);
            return err;
        }


        int do_extract(unzFile uf, int opt_extract_without_path, int opt_overwrite, const char* password)
        {
            uLong i;
            unz_global_info64 gi;
            int err;
            FILE* fout=NULL;

            err = unzGetGlobalInfo64(uf,&gi);
            if (err!=UNZ_OK)
                printf("error %d with zipfile in unzGetGlobalInfo \n",err);

            for (i=0;i<gi.number_entry;i++)
            {
                if (do_extract_currentfile(uf,&opt_extract_without_path,
                                              &opt_overwrite,
                                              password) != UNZ_OK)
                    break;

                if ((i+1)<gi.number_entry)
                {
                    err = unzGoToNextFile(uf);
                    if (err!=UNZ_OK)
                    {
                        printf("error %d with zipfile in unzGoToNextFile\n",err);
                        break;
                    }
                }
            }

            return 0;
        }

        int do_extract_onefile(unzFile uf, const char* filename, int opt_extract_without_path, int opt_overwrite, const char* password)
        {
            int err = UNZ_OK;
            if (unzLocateFile(uf,filename,CASESENSITIVITY)!=UNZ_OK)
            {
                printf("file %s not found in the zipfile\n",filename);
                return 2;
            }

            if (do_extract_currentfile(uf,&opt_extract_without_path,
                                              &opt_overwrite,
                                              password) == UNZ_OK)
                return 0;
            else
                return 1;
        }
    }//namespace _private_

    //namespace zip
    Archive::Archive()
        : password(""),
        error(""),
        optCompressLevel(Z_DEFAULT_COMPRESSION){
    }

    Archive::~Archive(){
        if (buff != NULL){
            delete[] buff;
            buff = NULL;
        }
    }

    Archive* Archive::set(Archive::Conf conf, int val){
        switch (conf){
            case Archive::Conf::UNZIP_OVERWRITE:
            case Archive::Conf::ZIP_OVERWRITE:{
                optOverwrite = (val == 1) ? 1 : 2;
            }break;
            case Archive::Conf::ZIP_APPEND:{
                optOverwrite = (val == 1) ? 2 : 1;
            }break;
            case Archive::Conf::COMPRESS_LEVEL:{
                optCompressLevel = (val < 0) ? 0 : ((val > 9) ? 9 : val);                    
            }break;
            default:{
            }
        }
        return this;
    }

    Archive* Archive::set(Archive::Conf conf, const std::string& val){
        switch (conf){
            case Archive::Conf::PASSWORD:{
                password = val;
            }break;
            default:{
            }
        }
        return this;
    }

    int Archive::get(Archive::Conf conf) const{
        switch (conf){
            case Archive::Conf::UNZIP_OVERWRITE:
            case Archive::Conf::ZIP_OVERWRITE:{
                return (optOverwrite == 1 ? 1 : 0);
            }break;
            case Archive::Conf::ZIP_APPEND:{
                return (optOverwrite == 2 ? 1 : 0);
            }break;
            case Archive::Conf::COMPRESS_LEVEL:{
                return optCompressLevel;
            }break;
            default:{
            }
        }
        return -1;
    }

    std::string Archive::get(Archive::Conf conf, bool _/*ignore this argument*/) const{
        switch (conf){
            case Archive::Conf::PASSWORD:{
                return password;
            }break;
            default:{
            }
        }
        return "";
    }

    std::string Archive::errMsg() const{
        return error;
    }

    int Archive::do_compress(const std::string& dst, const std::map<std::string, std::string>& srcMap){
        if(buff == NULL){
            buff = new char[buffSize];
            memset(buff, 0, buffSize);
        }
        zipFile zfile;
        int err = ZIP_OK;
#ifdef USEWIN32IOAPI
        zlib_filefunc64_def ffunc;
        fill_win32_filefunc64A(&ffunc);
        zfile = zipOpen2_64(dst.c_str(), (optOverwrite == 2) ? 2 : 0, NULL, &ffunc);
#else
        zfile = zipOpen64(dst.c_str(), (optOverwrite == 2) ? 2 : 0);
#endif

        if (zfile == NULL){
            error += "failed to open: " + dst + "\n";
            return ZIP_ERRNO;
        }

        for (auto iter = srcMap.begin(); iter != srcMap.end(); iter++){
            const char* filenameOnDisk = iter->first.c_str();
            const char* filenameInZip = iter->second.c_str();
            FILE* fin;
            int sizeRead;
            zip_fileinfo zf_info;
            unsigned long crcFile = 0;

            //Check if it's a big file
            int zip64 = zip::_private_::isLargeFile(filenameInZip);

            //Get file's date information
            zf_info.tmz_date.tm_sec = zf_info.tmz_date.tm_min = zf_info.tmz_date.tm_hour =
                zf_info.tmz_date.tm_mday = zf_info.tmz_date.tm_mon = zf_info.tmz_date.tm_year = 0;
            zf_info.dosDate = 0;
            zf_info.internal_fa = 0;
            zf_info.external_fa = 0;
            zip::_private_::filetime((char*)filenameInZip, &zf_info.tmz_date, &zf_info.dosDate);

            //Calculate CRC if password is required
            if (!password.empty()){
                err = zip::_private_::getFileCrc(filenameInZip, buff, buffSize, &crcFile);
                if (err != ZIP_OK) {
                    error += "failed to calculate CRC32 for file:" + std::string(filenameInZip) + "\n";
                    return err;
                }
            }

            //Remove the leading slash
            if (filenameInZip[0] == '\\' || filenameInZip[0] == '/'){
                filenameInZip++;
            }

            err = zipOpenNewFileInZip3_64(zfile, filenameInZip, &zf_info,
                            NULL, 0, NULL, 0, NULL,
                            (optCompressLevel != 0) ? Z_DEFLATED : 0,
                            optCompressLevel, 0,
                            -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                            password.empty() ? NULL : password.c_str(), crcFile, zip64);
            if (err != ZIP_OK) {
                error += std::string("failed to open [") + filenameInZip + "] in zip file\n";
                return err;
            }

            fin = FOPEN_FUNC(filenameOnDisk, "rb");
            if (fin == NULL) {
                error += std::string("failed to open file [") + filenameOnDisk + "] on disk for reading\n";
                continue;
                //return ZIP_ERRNO;
            }

            do {
                err = ZIP_OK;
                sizeRead = (int)fread(buff, 1, buffSize, fin);
                if (sizeRead < buffSize && feof(fin) == 0) {
                    error += std::string("failed to read file:") + filenameInZip + "\n";
                    return ZIP_ERRNO;
                }
                if (sizeRead > 0) {
                    err = zipWriteInFileInZip(zfile, buff, sizeRead);
                    if (err < 0) {
                        error += std::string("failed to write file [") + filenameInZip + "] into zip file\n";
                        return err;
                    }
                }
            } while (err == ZIP_OK && sizeRead > 0);

            if (fin) {
                fclose(fin);
            }

            if (err < 0) {
                return ZIP_ERRNO;
            } else {
                err = zipCloseFileInZip(zfile);
                if (err != ZIP_OK) {
                    error += std::string("failed to close file [") + filenameInZip + "] in zip file\n";
                    return err;
                }
            }
        }// end of processing one source file
        err = zipClose(zfile, NULL);
        if (err != ZIP_OK) {
            error += std::string("failed to close zip file:") + dst + "\n";
        }
        return ZIP_OK;
    }

    int Archive::compress(const std::vector<std::string>& srcList, const std::string& dst){
        std::map<std::string, std::string> srcMap;
        std::vector<std::string> files;
        std::string pathInZip;
        for(auto iter = srcList.begin(); iter != srcList.end(); iter++){
            std::string currSrc = *iter;
            files.clear();
            findAllFiles(currSrc, &files, true);
            for(int i = 0; i < files.size(); i++){
                pathInZip.clear();
                if (optExcludePath == 1){
                    if(isDir(currSrc)){
                        pathInZip = getRelativePath(currSrc, files[i]);
                    }else{
                        pathInZip = currSrc.substr(currSrc.find_last_of("\\/") + 1);
                    }
                }
                srcMap.insert(std::make_pair(files[i], pathInZip));
            }
        }
        return do_compress(dst, srcMap);
    }

    int Archive::uncompress(const std::string& zipPath,
                            const std::string& dst,
                            const std::vector<std::string> &srcList /*= std::vector<std::string>()*/){
        int opt_do_extract_withoutpath = 0;
        int opt_overwrite = 1;
#ifdef USEWIN32IOAPI
        zlib_filefunc64_def ffunc;
        fill_win32_filefunc64A(&ffunc);
        unzFile unzfile = unzOpen2_64(zipPath.c_str(),&ffunc);
#else
        unzFile unzfile = unzOpen64(zipPath.c_str());
#endif
        //whether the zip file cant be opened
        if(unzfile == NULL){
            error += "failed to open zip file:" + zipPath +"\n";
            return UNZ_ERRNO;
        }

        //create directory if dst doesn't exist
        zip::_private_::makedir(dst.c_str());
#ifdef _WIN32
        if (_chdir(dst.c_str()))
#else
        if (chdir(dst.c_str()))
#endif
        {
          error += "failed to change current directory to:" + dst + "\n";
          unzClose(unzfile);
          return UNZ_ERRNO;
        }

        if(srcList.empty()){ //extract all files zipped
            if(0 != zip::_private_::do_extract(unzfile,
                                               opt_do_extract_withoutpath,
                                               opt_overwrite,
                                               password.empty() ? NULL : password.c_str())){
                error += "failed to call extract zip file\n";
                unzClose(unzfile);
                return UNZ_ERRNO;
            }
        }else{
            for(int i = 0; i < srcList.size(); i++){
                if(0 != zip::_private_::do_extract_onefile(unzfile,
                                                           srcList[i].c_str(),
                                                           opt_do_extract_withoutpath,
                                                           opt_overwrite,
                                                           password.empty() ? NULL : password.c_str())){
                    error += "failed to call extract zip file\n";
                }
            }
        }
        unzClose(unzfile);
        return UNZ_OK;
    }

    int Archive::getFilesZipped(const std::string& zipPath, std::vector<std::string>* infos){
#ifdef USEWIN32IOAPI
        zlib_filefunc64_def ffunc;
        fill_win32_filefunc64A(&ffunc);
        unzFile unzfile = unzOpen2_64(zipPath.c_str(),&ffunc);
#else
        unzFile unzfile = unzOpen64(zipPath.c_str());
#endif
        uLong i;
        unz_global_info64 gi;
        int err;
        err = unzGetGlobalInfo64(unzfile, &gi);
        if (err != UNZ_OK){
            error += std::string("failed to get unzip global info of [") + zipPath + "]\n";
            return err;
        }

        for(i = 0; i <gi.number_entry; i++){
            char filename_inzip[256];
            unz_file_info64 file_info;
            uLong ratio=0;
            const char *string_method;
            char charCrypt=' ';
            err = unzGetCurrentFileInfo64(unzfile, &file_info, filename_inzip, sizeof(filename_inzip), NULL, 0, NULL, 0);
            if (err!=UNZ_OK){
                error += std::string("failed to call unzGetCurrentFileInfo()\n");
                break;
            }
            infos->push_back(std::string(filename_inzip));

            // display a '*' if the file is crypted
            if ((file_info.flag & 1) != 0){
                charCrypt='*';
            }

            if ((i+1) < gi.number_entry){
                err = unzGoToNextFile(unzfile);
                if (err != UNZ_OK){
                    error += "failed to call unzGoToNextFile()";
                    break;
                }
            }
        }
        return UNZ_OK;
    }

    std::string Archive::getRelativePath(const std::string& dirpath_, const std::string& filepath){
        std::string dirpath = dirpath_;
        if(dirpath.empty() || filepath.empty()){
            return "";
        }
        if(dirpath.find_last_of("\\/") == (dirpath.size()-1)){
            dirpath.erase(dirpath.size()-1);
        }
        std::string dirName = dirpath.substr(dirpath.find_last_of("\\/")+1);
        std::string result = filepath;
        result.replace(0, dirpath.length(), dirName);
        return result;
    }

    bool Archive::isDir(const std::string& path){
#ifdef _WIN32
        long hFile = 0;
        //文件信息
        struct _finddata_t fileInfo;
        hFile = _findfirst(path.c_str(), &fileInfo);
        if(hFile != -1 && (fileInfo.attrib & _A_SUBDIR)){
            return true;
        }
        return false;
#endif
    }

    void Archive::findAllFiles(const std::string& startPath, std::vector<std::string>* results, bool recur /*= true*/){
#ifdef _WIN32
        if(!isDir(startPath)){
            results->push_back(startPath);
            return;
        }
        long hFile = 0;
        //文件信息
        struct _finddata_t fileInfo;
        std::string path;
        if((hFile = _findfirst(path.assign(startPath).append("\\*").c_str(), &fileInfo)) !=  -1){
            do{
                //如果是目录, 迭代之, 如果不是, 加入列表
                if((fileInfo.attrib & _A_SUBDIR) && recur){
                    if(strcmp(fileInfo.name, ".") != 0  &&  strcmp(fileInfo.name,"..") != 0){
                        findAllFiles(path.assign(startPath).append("\\").append(fileInfo.name), results, recur);
                    }
                } else {
                    results->push_back(path.assign(startPath).append("\\").append(fileInfo.name));
                }
            }while(_findnext(hFile, &fileInfo) == 0);
            _findclose(hFile);
        }
#endif
    }
}
