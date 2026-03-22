#pragma once
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <memory>
#include <stdexcept>
#include <fstream>
#include "User.h"
#include "Message.h"

class ChatManager {
private:
    std::map<std::string, std::shared_ptr<User>> users;
    std::shared_ptr<User> currentUser;
    std::vector<Message> allMessages;  // Централизованное хранение всех сообщений
    mutable std::mutex mtx;

    // Работа с файлами
    void loadUsersFromFile();
    void saveUsersToFile() const;
    void loadMessagesFromFile();
    void saveMessagesToFile() const;
    std::string getUsersFilename() const { return "users.txt"; }
    std::string getMessagesFilename() const { return "messages.txt"; }

    // Вспомогательные методы
    bool userExists(const std::string& login) const;
    void validateUserInput(const std::string& login, const std::string& password, const std::string& name) const;
    std::vector<Message> getMessagesForUser(const std::string& userLogin, bool onlyPrivate) const;

public:
    ChatManager();
    ~ChatManager();  // Для сохранения данных при завершении

    void registerUser(const std::string& login, const std::string& password, const std::string& name);
    bool login(const std::string& login, const std::string& password);
    void logout();
    bool isLoggedIn() const;
    std::string getCurrentUserName() const;

    void sendPrivateMessage(const std::string& recipientLogin, const std::string& message);
    void sendPublicMessage(const std::string& message);

    std::vector<std::string> getPrivateMessagesForCurrentUser() const;
    std::vector<std::string> getPublicMessagesForCurrentUser() const;
    std::vector<std::string> getAllUsersExceptCurrent() const;

    void clearCurrentUserMessages();
    bool doesUserExist(const std::string& login) const;
};