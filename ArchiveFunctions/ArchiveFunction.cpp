#include "ArchiveFunction.h"

template<typename T>
using auto_cleanup = unique_ptr<T, std::function<void(T*)>>;

string ArchiveFunction::lastWordDirectory(string word) {
    vector<string> vec;

    string myText = word;
    istringstream iss(myText);
    string token;
    while (getline(iss, token, '\\')) vec.push_back(token);

    if (vec[vec.size() - 2] == tmp_path) return vec[vec.size() - 1];
    else if (vec[vec.size() - 3] == tmp_path) return vec[vec.size() - 2] + "\\" + vec[vec.size() - 1];
    else if (vec[vec.size() - 4] == tmp_path) return vec[vec.size() - 3] + "\\" + vec[vec.size() - 2] + "\\" + vec[vec.size() - 1];
    else return vec[vec.size() - 4] + "\\" + vec[vec.size() - 3] + "\\" + vec[vec.size() - 2] + "\\" + vec[vec.size() - 1];
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
    string s;
    int dist;
    // если в пути к файлу нет точки то нужно его удалить (значит это не файл, а папка в которой они лежат)
    for (auto it = vec.begin(); it != vec.end(); ++it) {
        dist = 0;
        s = *it;
        char arr[255];
        strcpy_s(arr, s.c_str());
        for (int i = 0; i < strlen(arr); i++) {
            if (arr[i] == '.') dist = 1;
        }
        if(!dist) it = vec.erase(it);
    }
}

void ArchiveFunction::extract(string archive_path, string save_path) {
    struct archive_entry* entry;
    string destination = save_path;
    int flags;
    int r;

    flags = ARCHIVE_EXTRACT_TIME;
    flags |= ARCHIVE_EXTRACT_PERM;
    flags |= ARCHIVE_EXTRACT_ACL;
    flags |= ARCHIVE_EXTRACT_FFLAGS;

    auto_cleanup<struct archive> a(archive_read_new(), [](struct archive* a) { archive_read_close(a); archive_read_free(a); });
    archive_read_support_format_all(&*a);
    archive_read_support_compression_all(&*a);
    auto_cleanup<struct archive> ext(archive_write_disk_new(), [](struct archive* ext) { archive_write_close(ext); archive_write_free(ext); });
    archive_write_disk_set_options(&*ext, flags);
    archive_write_disk_set_standard_lookup(&*ext);
    if ((r = archive_read_open_filename(&*a, archive_path.c_str(), 10240)))
        exit(1);
    for (;;) {
        r = archive_read_next_header(&*a, &entry);
        if (r == ARCHIVE_EOF)
            break;
        if (r < ARCHIVE_OK)
            fprintf(stderr, "%s\n", archive_error_string(&*a));
        if (r < ARCHIVE_WARN)
            exit(1);
        const char* currentFile = archive_entry_pathname(entry);
        const string fullOutputPath = destination + "\\" + currentFile;
        archive_entry_set_pathname(entry, fullOutputPath.c_str());
        r = archive_write_header(&*ext, entry);
        if (r < ARCHIVE_OK)
            fprintf(stderr, "%s\n", archive_error_string(&*ext));
        else if (archive_entry_size(entry) > 0) {
            r = copy_data(&*a, &*ext);
            if (r < ARCHIVE_OK)
                fprintf(stderr, "%s\n", archive_error_string(&*ext));
            if (r < ARCHIVE_WARN)
                exit(1);
        }
        r = archive_write_finish_entry(&*ext);
        if (r < ARCHIVE_OK)
            fprintf(stderr, "%s\n", archive_error_string(&*ext));
        if (r < ARCHIVE_WARN)
            exit(1);
    }
}

void ArchiveFunction::extractArchive() {
    openFile();

    if (GetOpenFileName(&ofn) == TRUE) {
        str = ofn.lpstrFile;
        path = tmp_path = string(str.begin(), str.end());

        saveFile();

        if (GetSaveFileName(&ofn) == TRUE) {
            str = ofn.lpstrFile;
            outname = string(str.begin(), str.end());

            extract(path, outname);
        }
    }    
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
        archive_read_open_filename(&*b, global_archive.c_str(), 10240);
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
    filesystem::rename("arch.zip", global_archive.c_str());
    readArchive(global_archive);
}

void ArchiveFunction::openFile() {
    ofn.lStructSize = sizeof(ofn);
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

void ArchiveFunction::openArchive() {
    ofn.lStructSize = sizeof(ofn);
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

void ArchiveFunction::writeArchiveSingle() {
    Flag = 0;
    openFile();

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

void ArchiveFunction::writeArchiveDirectory() {
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

void ArchiveFunction::selectFile() {
    openFile();

    if (GetOpenFileName(&ofn) == TRUE) {
        str = ofn.lpstrFile;
        glob_path = string(str.begin(), str.end());
    }
}

void ArchiveFunction::selectArchive() {
    openArchive();

    if (GetOpenFileName(&ofn) == TRUE) {
        str = ofn.lpstrFile;
        global_archive = string(str.begin(), str.end());
    }
}

void ArchiveFunction::DoArchiveParam(std::vector<float>& size_, std::vector<std::string>& name_, std::vector<int>& height_) {
    openArchive();

    if (GetOpenFileName(&ofn) == TRUE) {
        str = ofn.lpstrFile;
        global_archive = string(str.begin(), str.end());

        amount_size_files = sizeFiles(global_archive, number);
        sort(amount_size_files.begin(), amount_size_files.end());

        fstream file(global_archive);
        file.seekg(0, std::ios::end);
        max_size = file.tellg();
        file.close();
        max_size /= 1024;
        max_size /= 1024;

        for (auto arg : amount_size_files) {
            size.push_back(arg.first);
            name.push_back(arg.second);
        }
        ++number;

        size.push_back(max_size);
        name.push_back(lastWordFile(global_archive));

        ratioCpp(max_size, size, height);

        for (int i = 0; i < size.size(); ++i) size_.push_back(size[i]);
        for (int i = 0; i < name.size(); ++i) name_.push_back(name[i]);
        for (int i = 0; i < height.size(); ++i) height_.push_back(height[i]);
    }
}

void ArchiveFunction::DoFileParam(std::vector<float>& size_, std::vector<std::string>& name_, std::vector<int>& height_) {
    openArchive();

    if (GetOpenFileName(&ofn) == TRUE) {
        str = ofn.lpstrFile;
        global_archive = string(str.begin(), str.end());

        amount_size_files = sizeFiles(global_archive, number);
        sort(amount_size_files.begin(), amount_size_files.end());

        for (auto arg : amount_size_files) {
            size.push_back(arg.first);
            name.push_back(arg.second);
        }

        auto it = size.begin();
        advance(it, size.size() - 1);
        max_size = *it;

        ratioCs(max_size, size, height);
        for (int i = 0; i < size.size(); ++i) size_.push_back(size[i]);
        for (int i = 0; i < name.size(); ++i) name_.push_back(name[i]);
        for (int i = 0; i < height.size(); ++i) height_.push_back(height[i]);
    }
}

vector<pair<float, string>> ArchiveFunction::sizeFiles(const string path, int& count) {
    struct archive_entry* entry;
    int r;
    float size;
    string text;
    vector <pair<float, string>> files;

    auto_cleanup<struct archive> a(archive_read_new(), [](struct archive* a) { archive_read_free(a); });
    archive_read_support_filter_all(&*a);
    archive_read_support_format_all(&*a);
    r = archive_read_open_filename(&*a, path.c_str(), 10240);
    if (r != ARCHIVE_OK)
        exit(1);
    while (archive_read_next_header(&*a, &entry) == ARCHIVE_OK) {
        text = archive_entry_pathname(entry);
        size = archive_entry_size(entry);
        size /= 1024;
        size /= 1024;
        files.push_back(make_pair(size, text));
        count++;
        archive_read_data_skip(&*a);
    }
    return files;
}

void ArchiveFunction::ratioCs(float max, vector<float> vec, vector<int>& hg) {
    vector<int>mnb;
    for (auto i : vec) {
        mnb.push_back(i / max * 10);
    }
    for (auto i : mnb) {
        hg.push_back(i);
    }
}

LRESULT CALLBACK ArchiveFunction::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    ArchiveFunction* me = reinterpret_cast<ArchiveFunction*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    if (me) return me->realWndProc(hwnd, msg, wParam, lParam);
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK ArchiveFunction::realWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg)
    {
    case WM_PAINT:
        hdc = BeginPaint(hwnd, &ps);
        SelectObject(hdc, hFont);
        for (int i = number - 1; i >= 0; --i) {
            r.top = 250;
            r.left = lft;
            r.right = rht;
            r.bottom = height[i];

            wstring tmp_name(name[i].begin(), name[i].end());
            TextOut(hdc, rht, height[i] - 30, tmp_name.c_str(), tmp_name.size());

            string s = to_string(size[i]);
            wstring tmp2(s.begin(), s.end());
            TextOut(hdc, rht, 250, tmp2.c_str(), tmp2.size());

            FillRect(hdc, &r, HBRUSH(CreateSolidBrush(RGB(27, 58, 194))));
            lft += 80;
            rht += 80;
        }

        EndPaint(hwnd, &ps);
        UpdateWindow(hwnd);
        ShowWindow(hwnd, SW_SHOW);
        break;

    case WM_CLOSE:
        PostQuitMessage(0);
        break;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

void ArchiveFunction::ratioCpp(float max, vector<float> vec, vector<int>& hg) {
    vector<int>mnb;
    for (auto i : vec) {
        mnb.push_back(i / max * 100);
    }
    for (auto i : mnb) {
        hg.push_back(250 - (i * 2));
    }
    sort(hg.begin(), hg.end(), greater<>());
}

void ArchiveFunction::Draw() {
    WNDCLASS my_wndclass_struct;

    my_wndclass_struct.style = CS_OWNDC;

    my_wndclass_struct.lpfnWndProc = WndProc;

    my_wndclass_struct.cbClsExtra = 0;
    my_wndclass_struct.cbWndExtra = 0;
    my_wndclass_struct.hInstance = GetModuleHandle(NULL);
    my_wndclass_struct.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    my_wndclass_struct.hCursor = LoadCursor(NULL, IDC_ARROW);
    my_wndclass_struct.hbrBackground = (HBRUSH)(6);
    my_wndclass_struct.lpszMenuName = NULL;
    my_wndclass_struct.lpszClassName = TEXT("Diagram_C++");

    RegisterClass(&my_wndclass_struct);

    HWND hwnd = CreateWindow(
        TEXT("Diagram_C++"),
        TEXT("Diagram"),
        WS_OVERLAPPEDWINDOW,
        100,
        100,
        800,
        350,
        NULL,
        NULL,
        GetModuleHandle(NULL),
        NULL
    );
    HWND: SetWindowLongPtr(hwnd, GWLP_USERDATA, (long)this);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
}