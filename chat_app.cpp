#include <iostream>
#include <limits>
#include <string>
#include "Chat_Manager.h"

// Функция безопасного ввода целого числа
int getIntInput(const std::string& prompt) {
    int value;
    while (true) {
        std::cout << prompt;
        std::cin >> value;

        if (std::cin.fail()) {
            std::cin.clear(); // сбрасываем флаг ошибки
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Ошибка: введите число.\n";
        }
        else {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return value;
        }
    }
}

void clearInput() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void showMainMenu() {
    std::cout << "\n=== Главное меню ===\n";
    std::cout << "1. Регистрация\n";
    std::cout << "2. Вход\n";
    std::cout << "3. Выход\n";
}

void showUserMenu() {
    std::cout << "\n=== Меню пользователя ===\n";
    std::cout << "1. Отправить личное сообщение\n";
    std::cout << "2. Отправить сообщение всем\n";
    std::cout << "3. Посмотреть личные сообщения\n";
    std::cout << "4. Посмотреть публичные сообщения\n";
    std::cout << "5. Список пользователей\n";
    std::cout << "6. Очистить мои сообщения\n";
    std::cout << "7. Выйти из аккаунта\n";
}

int main() {
    ChatManager manager;
    bool running = true;

    while (running) {
        if (!manager.isLoggedIn()) {
            showMainMenu();
            int choice = getIntInput("Выберите действие: ");

            switch (choice) {
            case 1: { // Регистрация
                std::string login, password, name;
                std::cout << "Введите логин: ";
                std::getline(std::cin, login);
                std::cout << "Введите пароль: ";
                std::getline(std::cin, password);
                std::cout << "Введите имя: ";
                std::getline(std::cin, name);

                try {
                    manager.registerUser(login, password, name);
                    std::cout << "Регистрация успешна!\n";
                }
                catch (const std::exception& e) {
                    std::cout << "Ошибка: " << e.what() << "\n";
                }
                break;
            }
            case 2: { // Вход
                std::string login, password;
                std::cout << "Логин: ";
                std::getline(std::cin, login);
                std::cout << "Пароль: ";
                std::getline(std::cin, password);

                if (manager.login(login, password)) {
                    std::cout << "Добро пожаловать, " << manager.getCurrentUserName() << "!\n";
                }
                else {
                    std::cout << "Неверный логин или пароль.\n";
                }
                break;
            }
            case 3: // Выход из программы
                running = false;
                std::cout << "До свидания!\n";
                break;
            default:
                std::cout << "Неверный выбор, попробуйте снова.\n";
            }
        }
        else {
            showUserMenu();
            int choice = getIntInput("Выберите действие: ");

            switch (choice) {
            case 1: { // Личное сообщение
                std::string recipient, message;
                std::cout << "Введите логин получателя: ";
                std::getline(std::cin, recipient);
                std::cout << "Введите сообщение: ";
                std::getline(std::cin, message);

                try {
                    manager.sendPrivateMessage(recipient, message);
                    std::cout << "Сообщение отправлено.\n";
                }
                catch (const std::exception& e) {
                    std::cout << "Ошибка: " << e.what() << "\n";
                }
                break;
            }
            case 2: { // Публичное сообщение
                std::string message;
                std::cout << "Введите сообщение для всех: ";
                std::getline(std::cin, message);

                try {
                    manager.sendPublicMessage(message);
                    std::cout << "Сообщение отправлено всем.\n";
                }
                catch (const std::exception& e) {
                    std::cout << "Ошибка: " << e.what() << "\n";
                }
                break;
            }
            case 3: { // Личные сообщения
                auto msgs = manager.getPrivateMessagesForCurrentUser();
                if (msgs.empty()) {
                    std::cout << "Личных сообщений нет.\n";
                }
                else {
                    std::cout << "\n=== Личные сообщения ===\n";
                    for (const auto& msg : msgs) {
                        std::cout << msg << "\n";
                    }
                }
                break;
            }
            case 4: { // Публичные сообщения
                auto msgs = manager.getPublicMessagesForCurrentUser();
                if (msgs.empty()) {
                    std::cout << "Публичных сообщений нет.\n";
                }
                else {
                    std::cout << "\n=== Публичные сообщения ===\n";
                    for (const auto& msg : msgs) {
                        std::cout << msg << "\n";
                    }
                }
                break;
            }
            case 5: { // Список пользователей
                auto users = manager.getAllUsersExceptCurrent();
                if (users.empty()) {
                    std::cout << "Других пользователей нет.\n";
                }
                else {
                    std::cout << "\n=== Другие пользователи ===\n";
                    for (const auto& u : users) {
                        std::cout << u << "\n";
                    }
                }
                break;
            }
            case 6: { // Очистить сообщения
                manager.clearCurrentUserMessages();
                std::cout << "Ваши сообщения удалены.\n";
                break;
            }
            case 7: // Выход из аккаунта
                manager.logout();
                std::cout << "Вы вышли из аккаунта.\n";
                break;
            default:
                std::cout << "Неверный выбор, попробуйте снова.\n";
            }
        }
    }

    return 0;
}