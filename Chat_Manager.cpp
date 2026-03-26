#define _CRT_SECURE_NO_WARNINGS  // отключает предупреждения о localtime в Visual Studio

#include "Chat_Manager.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <iomanip>

ChatManager::ChatManager() {
    loadUsersFromFile();
    loadMessagesFromFile();
}

ChatManager::~ChatManager() {
    saveUsersToFile();
    saveMessagesToFile();
}

void ChatManager::loadUsersFromFile() {
    std::ifstream file(getUsersFilename());
    if (!file.is_open()) {
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;

        std::stringstream ss(line);
        std::string login, password, name;

        if (std::getline(ss, login, ':') &&
            std::getline(ss, password, ':') &&
            std::getline(ss, name)) {
            users[login] = std::make_shared<User>(login, password, name);
        }
    }
    file.close();
}

void ChatManager::saveUsersToFile() const {
    std::ofstream file(getUsersFilename());
    if (!file.is_open()) {
        return;
    }

    for (const auto& [login, user] : users) {
        file << user->getLogin() << ":"
            << user->getPassword() << ":"
            << user->getName() << "\n";
    }
    file.close();
}

void ChatManager::loadMessagesFromFile() {
    std::ifstream file(getMessagesFilename());
    if (!file.is_open()) {
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;

        std::stringstream ss(line);
        std::string sender, recipient, text, timeStr;

        if (std::getline(ss, sender, ':') &&
            std::getline(ss, recipient, ':') &&
            std::getline(ss, text, ':') &&
            std::getline(ss, timeStr)) {
            std::time_t time = static_cast<std::time_t>(std::stoll(timeStr));
            Message msg(sender, recipient, text, time);
            allMessages.push_back(msg);
        }
    }
    file.close();
}

void ChatManager::saveMessagesToFile() const {
    std::ofstream file(getMessagesFilename());
    if (!file.is_open()) {
        return;
    }

    for (const auto& msg : allMessages) {
        file << msg.getSender() << ":"
            << msg.getRecipient() << ":"
            << msg.getText() << ":"
            << msg.getTime() << "\n";
    }
    file.close();
}

std::string ChatManager::formatTime(std::time_t time) const {
    char buffer[26];
    struct tm* timeInfo = std::localtime(&time);
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeInfo);
    return std::string(buffer);
}

bool ChatManager::userExists(const std::string& login) const {
    return users.find(login) != users.end();
}

void ChatManager::validateUserInput(const std::string& login,
    const std::string& password,
    const std::string& name) const {
    if (login.empty() || password.empty() || name.empty()) {
        throw std::invalid_argument("All fields (login, password, name) must be filled");
    }

    if (login.length() < 3) {
        throw std::invalid_argument("Login must be at least 3 characters long");
    }

    if (password.length() < 4) {
        throw std::invalid_argument("Password must be at least 4 characters long");
    }
}

std::vector<Message> ChatManager::getMessagesForUser(const std::string& userLogin,
    bool onlyPrivate) const {
    std::vector<Message> userMessages;

    for (const auto& msg : allMessages) {
        bool isRecipient = (msg.getRecipient() == userLogin);
        bool isSender = (msg.getSender() == userLogin);

        if (onlyPrivate) {
            if ((isRecipient || isSender) && msg.getRecipient() != "ALL") {
                userMessages.push_back(msg);
            }
        }
        else {
            if (msg.getRecipient() == "ALL") {
                userMessages.push_back(msg);
            }
        }
    }

    return userMessages;
}

void ChatManager::registerUser(const std::string& login,
    const std::string& password,
    const std::string& name) {
    std::lock_guard<std::mutex> lock(mtx);

    validateUserInput(login, password, name);

    if (userExists(login)) {
        throw std::runtime_error("User with this login already exists");
    }

    users[login] = std::make_shared<User>(login, password, name);
    saveUsersToFile();
}

bool ChatManager::login(const std::string& login, const std::string& password) {
    std::lock_guard<std::mutex> lock(mtx);

    auto it = users.find(login);
    if (it != users.end() && it->second->checkPassword(password)) {
        currentUser = it->second;
        return true;
    }
    return false;
}

void ChatManager::logout() {
    std::lock_guard<std::mutex> lock(mtx);
    currentUser.reset();
}

bool ChatManager::isLoggedIn() const {
    std::lock_guard<std::mutex> lock(mtx);
    return currentUser != nullptr;
}

std::string ChatManager::getCurrentUserName() const {
    std::lock_guard<std::mutex> lock(mtx);
    if (currentUser) {
        return currentUser->getName();
    }
    return "";
}

void ChatManager::sendPrivateMessage(const std::string& recipientLogin,
    const std::string& message) {
    std::lock_guard<std::mutex> lock(mtx);

    if (!currentUser) {
        throw std::runtime_error("No user logged in");
    }

    if (message.empty()) {
        throw std::invalid_argument("Message cannot be empty");
    }

    if (!userExists(recipientLogin)) {
        throw std::runtime_error("Recipient does not exist");
    }

    Message msg(currentUser->getLogin(), recipientLogin, message, std::time(nullptr));
    allMessages.push_back(msg);
    saveMessagesToFile();
}

void ChatManager::sendPublicMessage(const std::string& message) {
    std::lock_guard<std::mutex> lock(mtx);

    if (!currentUser) {
        throw std::runtime_error("No user logged in");
    }

    if (message.empty()) {
        throw std::invalid_argument("Message cannot be empty");
    }

    Message msg(currentUser->getLogin(), "ALL", message, std::time(nullptr));
    allMessages.push_back(msg);
    saveMessagesToFile();
}

std::vector<std::string> ChatManager::getPrivateMessagesForCurrentUser() const {
    std::lock_guard<std::mutex> lock(mtx);

    if (!currentUser) {
        return {};
    }

    std::vector<std::string> result;
    std::vector<Message> messages = getMessagesForUser(currentUser->getLogin(), true);

    for (const auto& msg : messages) {
        std::stringstream ss;
        if (msg.getSender() == currentUser->getLogin()) {
            ss << "[To " << msg.getRecipient() << "] "
                << formatTime(msg.getTime()) << ": " << msg.getText();
        }
        else {
            ss << "[From " << msg.getSender() << "] "
                << formatTime(msg.getTime()) << ": " << msg.getText();
        }
        result.push_back(ss.str());
    }

    return result;
}

std::vector<std::string> ChatManager::getPublicMessagesForCurrentUser() const {
    std::lock_guard<std::mutex> lock(mtx);

    if (!currentUser) {
        return {};
    }

    std::vector<std::string> result;
    std::vector<Message> messages = getMessagesForUser(currentUser->getLogin(), false);

    for (const auto& msg : messages) {
        std::stringstream ss;
        ss << "[" << msg.getSender() << "] "
            << formatTime(msg.getTime()) << ": " << msg.getText();
        result.push_back(ss.str());
    }

    return result;
}

std::vector<std::string> ChatManager::getAllUsersExceptCurrent() const {
    std::lock_guard<std::mutex> lock(mtx);

    std::vector<std::string> result;
    std::string currentLogin = currentUser ? currentUser->getLogin() : "";

    for (const auto& [login, user] : users) {
        if (login != currentLogin) {
            result.push_back(login + " (" + user->getName() + ")");
        }
    }

    return result;
}

void ChatManager::clearCurrentUserMessages() {
    std::lock_guard<std::mutex> lock(mtx);

    if (!currentUser) {
        return;
    }

    std::string currentLogin = currentUser->getLogin();

    allMessages.erase(
        std::remove_if(allMessages.begin(), allMessages.end(),
            [&currentLogin](const Message& msg) {
                return msg.getSender() == currentLogin ||
                    msg.getRecipient() == currentLogin;
            }),
        allMessages.end()
    );

    saveMessagesToFile();
}

bool ChatManager::doesUserExist(const std::string& login) const {
    std::lock_guard<std::mutex> lock(mtx);
    return userExists(login);
}