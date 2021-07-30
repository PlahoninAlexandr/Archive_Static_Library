#include "ArchiveFunction.h"

template<typename T>
using auto_cleanup = unique_ptr<T, std::function<void(T*)>>;

string ArchiveFunction::lastWordDirectory(string word) {
    vector<string> vec;

    string myText = word;
    istringstream iss(myText);
    string token;
    while (getline(iss, token, '\\')) vec.push_back(token);

    if (vec[vec.size() - 3] != tmp_path) return vec[vec.size() - 3] + "\\" + vec[vec.size() - 2] + "\\" + vec[vec.size() - 1];
    else if (vec[vec.size() - 2] != tmp_path) return vec[vec.size() - 2] + "\\" + vec[vec.size() - 1];
    else return vec[vec.size() - 1];
}

string ArchiveFunction::lastWordFile(string word) {
    vector<string> vec;

    string myText = word;
    istringstream iss(myText);
    string token;
    while (getline(iss, token, '\\')) vec.push_back(token);

    return vec[vec.size() - 1];
}

void ArchiveFunction::write_archive(const string outname_tmp, const vector<string> filename) {
    string outname = lastWordFile(outname_tmp);
    struct stat st;
    int len;
    string ss;

    auto_cleanup<struct archive> a(archive_write_new(), [](struct archive* a) { archive_write_close(a); archive_write_free(a); });
    archive_write_set_format_zip(&*a);
    archive_write_set_format_pax_restricted(&*a);
    archive_write_open_filename(&*a, outname.c_str());
    tmp_path = lastWordFile(tmp_path);

    for (auto i : filename) {
        stat(i.c_str(), &st);
        auto_cleanup<struct archive_entry> entry(archive_entry_new(), [](struct archive_entry* entry) { archive_entry_free(entry); });
        if (Flag == 1) archive_entry_set_pathname(&*entry, lastWordDirectory(i).c_str());
        else archive_entry_set_pathname(&*entry, lastWordFile(i).c_str());
        archive_entry_set_size(&*entry, st.st_size);
        archive_entry_set_filetype(&*entry, AE_IFREG);
        archive_entry_set_perm(&*entry, 0644);
        archive_write_header(&*a, &*entry);
        auto_cleanup<FILE> pFile(fopen(i.c_str(), "rb"), [](FILE* pFile) { fclose(pFile); });
        fseek(&*pFile, 0, SEEK_END);
        size_t size = ftell(&*pFile);
        ss.resize(size);
        rewind(&*pFile);
        len = fread(&ss[0], 1, size, &*pFile);
        while (len > 0) {
            archive_write_data(&*a, &ss[0], len);
            len = fread(&ss[0], 1, size, &*pFile);
        }
        ss.clear();
    }
    readArchive(outname_tmp);
}

void ArchiveFunction::readArchive(const string path) {
    struct archive_entry* entry;
    int r;
    string text;

    auto_cleanup<struct archive> a(archive_read_new(), [](struct archive* a) { archive_read_free(a); });
    archive_read_support_filter_all(&*a);
    archive_read_support_format_all(&*a);
    r = archive_read_open_filename(&*a, path.c_str(), 10240);
    if (r != ARCHIVE_OK)
        exit(1);
    while (archive_read_next_header(&*a, &entry) == ARCHIVE_OK) {
        text += archive_entry_pathname(entry);
        text += "\n";
        archive_read_data_skip(&*a);
    }
    MessageBoxA(NULL, text.c_str(), "List", MB_ICONINFORMATION | MB_OKCANCEL);
}

void ArchiveFunction::chekBrokenPath(vector<string>& vec) {
    auto iter = vec.begin();
    vector<string> second_vec(vec);
    string s;
    int tmp = 0;
    for (vector<string>::reverse_iterator it = second_vec.rbegin(); it != second_vec.rend(); ++it) {
        s = *it;
        size_t foundIndex = s.find(".");
        if (foundIndex != string::npos) continue;
        tmp = distance(second_vec.rbegin(), it);
        advance(iter, vec.size() - tmp - 1);
        iter = vec.erase(iter);
    }
}

void ArchiveFunction::extract(const char* filename) {
    struct archive* a;
    struct archive* ext;
    struct archive_entry* entry;
    string destination = "C:\\Users\\Mi Notebook\\Desktop\\UI_archive\\UI_archive\\TMP";
    int flags;
    int r;

    flags = ARCHIVE_EXTRACT_TIME;
    flags |= ARCHIVE_EXTRACT_PERM;
    flags |= ARCHIVE_EXTRACT_ACL;
    flags |= ARCHIVE_EXTRACT_FFLAGS;

    a = archive_read_new();
    archive_read_support_format_all(a);
    archive_read_support_compression_all(a);
    ext = archive_write_disk_new();
    archive_write_disk_set_options(ext, flags);
    archive_write_disk_set_standard_lookup(ext);
    if ((r = archive_read_open_filename(a, filename, 10240)))
        exit(1);
    for (;;) {
        r = archive_read_next_header(a, &entry);
        if (r == ARCHIVE_EOF)
            break;
        if (r < ARCHIVE_OK)
            fprintf(stderr, "%s\n", archive_error_string(a));
        if (r < ARCHIVE_WARN)
            exit(1);
        const char* currentFile = archive_entry_pathname(entry);
        const std::string fullOutputPath = destination + "\\" + currentFile;
        archive_entry_set_pathname(entry, fullOutputPath.c_str());
        r = archive_write_header(ext, entry);
        if (r < ARCHIVE_OK)
            fprintf(stderr, "%s\n", archive_error_string(ext));
        else if (archive_entry_size(entry) > 0) {
            r = copy_data(a, ext);
            if (r < ARCHIVE_OK)
                fprintf(stderr, "%s\n", archive_error_string(ext));
            if (r < ARCHIVE_WARN)
                exit(1);
        }
        r = archive_write_finish_entry(ext);
        if (r < ARCHIVE_OK)
            fprintf(stderr, "%s\n", archive_error_string(ext));
        if (r < ARCHIVE_WARN)
            exit(1);
    }
    archive_read_close(a);
    archive_read_free(a);
    archive_write_close(ext);
    archive_write_free(ext);
}

int ArchiveFunction::copy_data(struct archive* ar, struct archive* aw) {
    int r;
    const void* buff;
    size_t size;
    la_int64_t offset;

    for (;;) {
        r = archive_read_data_block(ar, &buff, &size, &offset);
        if (r == ARCHIVE_EOF)
            return (ARCHIVE_OK);
        if (r < ARCHIVE_OK)
            return (r);
        r = archive_write_data_block(aw, buff, size, offset);
        if (r < ARCHIVE_OK) {
            fprintf(stderr, "%s\n", archive_error_string(aw));
            return (r);
        }
    }
}

void ArchiveFunction::addFileInArchive() {
    struct archive_entry* entry;
    struct stat st;
    int len;
    string ss;

    unsigned int aUnzipBlobSize = 8192;
    char aUnzipBlob[8192];
    long int  aUnzipSize = 0;
    {
        auto_cleanup<struct archive> a(archive_write_new(), [](struct archive* a) { archive_write_free(a); });
        archive_write_set_format_zip(&*a);
        archive_write_set_format_pax_restricted(&*a);
        archive_write_open_filename(&*a, "arch.zip");

        auto_cleanup<struct archive> b(archive_read_new(), [](struct archive* b) { archive_read_free(b); });
        archive_read_support_filter_all(&*b);
        archive_read_support_format_all(&*b);
        archive_read_open_filename(&*b, glob_arch.c_str(), 10240);
        while (archive_read_next_header(&*b, &entry) == ARCHIVE_OK) {
            archive_write_header(&*a, entry);
            aUnzipSize = archive_read_data(&*b, aUnzipBlob, aUnzipBlobSize);
            while (aUnzipSize > 0) {
                archive_write_data(&*a, aUnzipBlob, aUnzipBlobSize);
                aUnzipSize = archive_read_data(&*b, aUnzipBlob, aUnzipBlobSize);
            }
            archive_write_finish_entry(&*a);
        }

        stat(glob_path.c_str(), &st);
        entry = archive_entry_new();
        archive_entry_set_pathname(entry, lastWordFile(glob_path).c_str());
        archive_entry_set_size(entry, st.st_size);
        archive_entry_set_filetype(entry, AE_IFREG);
        archive_entry_set_perm(entry, 0644);
        archive_write_header(&*a, entry);
        auto_cleanup<FILE> pFile(fopen(glob_path.c_str(), "rb"), [](FILE* pFile) { fclose(pFile); });
        fseek(&*pFile, 0, SEEK_END);
        size_t size = ftell(&*pFile);
        ss.resize(size);
        rewind(&*pFile);
        len = fread(&ss[0], 1, size, &*pFile);
        while (len > 0) {
            archive_write_data(&*a, &ss[0], len);
            len = fread(&ss[0], 1, size, &*pFile);
        }
        ss.clear();
        archive_entry_free(entry);
    }
    filesystem::rename("arch.zip", glob_arch.c_str());
    readArchive(glob_arch);
}

void ArchiveFunction::close(HWND hWnd1, HWND hWnd2) {
    SetWindowPos(hWnd1, HWND_NOTOPMOST, 0, 0, 0, 0, NULL);
    ShowWindow(hWnd1, SW_HIDE);
    EnableWindow(hWnd2, TRUE);
    SetWindowPos(hWnd2, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
}

void ArchiveFunction::openFile(HWND hWnd) {
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFile = file;
    ofn.nMaxFile = sizeof(file);
    ofn.lpstrFilter = _T("All\0*.*\0Text\0*.TXT\0");
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
}

void ArchiveFunction::openDirectory() {
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFile = file;
    ofn.nMaxFile = sizeof(file);
    ofn.lpstrFilter = _T("All\0*.*\0Text\0*.TXT\0");
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_ALLOWMULTISELECT | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER | OFN_ALLOWMULTISELECT;
    ofn.lpstrTitle = TEXT("title");
}

void ArchiveFunction::openArchive(HWND hWnd) {
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFile = file;
    ofn.nMaxFile = sizeof(file);
    ofn.lpstrFilter = _T("zip\0*.zip*\0Text\0*.TXT\0");
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
}

void ArchiveFunction::saveFile() {
    char szDefExt[260];
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = file;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = sizeof(file);
    ofn.lpstrFilter = L"(*.zip)\0*.zip\0";
    ofn.lpstrDefExt = (LPCWSTR)szDefExt;
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
}

void ArchiveFunction::writeArchiveSingle(HWND hWnd, wstring str, string path, vector<string> argv, string outname) {
    Flag = 0;
    openFile(hWnd);

    if (GetOpenFileName(&ofn) == TRUE) {
        str = ofn.lpstrFile;
        path = tmp_path = string(str.begin(), str.end());

        argv.push_back(path);

        saveFile();

        if (GetSaveFileName(&ofn) == TRUE) {
            str = ofn.lpstrFile;
            outname = string(str.begin(), str.end());

            write_archive(outname, argv);
        }
    }
}

void ArchiveFunction::writeArchiveDirectory(wstring str, string path, vector<string> argv, string outname) {
    Flag = 1;
    openDirectory();

    if (GetOpenFileName(&ofn) == true) {
        str = ofn.lpstrFile;
        path = tmp_path = string(str.begin(), str.end());

        for (const auto& entry : fs::recursive_directory_iterator(path))
            argv.push_back(entry.path().string());
        chekBrokenPath(argv);

        saveFile();

        if (GetSaveFileName(&ofn) == TRUE) {
            str = ofn.lpstrFile;
            outname = string(str.begin(), str.end());

            write_archive(outname, argv);
        }
    }
}

void ArchiveFunction::selectFile(HWND& hWnd, wstring& str) {
    openFile(hWnd);

    if (GetOpenFileName(&ofn) == TRUE) {
        str = ofn.lpstrFile;
        glob_path = string(str.begin(), str.end());
    }
}

void ArchiveFunction::selectArchive(HWND& hWnd, wstring& str) {
    openArchive(hWnd);

    if (GetOpenFileName(&ofn) == TRUE) {
        str = ofn.lpstrFile;
        glob_arch = string(str.begin(), str.end());
    }
}