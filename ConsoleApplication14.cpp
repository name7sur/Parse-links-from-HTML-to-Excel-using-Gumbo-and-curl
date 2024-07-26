#include <iostream>
#include <sstream>
#include <string>
#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <gumbo.h>
#include <xlsxwriter.h>
#include <vector>

// Функция для получения HTML-страницы
std::string fetchHtml(const std::string& url) {
    try {
        curlpp::Cleanup cleaner;
        curlpp::Easy request;

        request.setOpt(new curlpp::Options::Url(url));
        std::ostringstream response;
        request.setOpt(new curlpp::Options::WriteStream(&response));

        request.perform();
        return response.str();
    }
    catch (curlpp::RuntimeError& e) {
        std::cerr << "Runtime error: " << e.what() << std::endl;
    }
    catch (curlpp::LogicError& e) {
        std::cerr << "Logic error: " << e.what() << std::endl;
    }
    return "";
}

// Функция для рекурсивного парсинга HTML и извлечения ссылок
void parseHtml(GumboNode* node, std::vector<std::string>& links) {
    if (node->type != GUMBO_NODE_ELEMENT) {
        return;
    }

    if (node->v.element.tag == GUMBO_TAG_A) {
        GumboAttribute* href = gumbo_get_attribute(&node->v.element.attributes, "href");
        if (href) {
            links.push_back(href->value);
        }
    }

    GumboVector* children = &node->v.element.children;
    for (unsigned int i = 0; i < children->length; ++i) {
        parseHtml(static_cast<GumboNode*>(children->data[i]), links);
    }
}

int main() {
    // URL страницы
    std::string url = "https://libxlsxwriter.github.io/";

    // Получение HTML-контента
    std::string html = fetchHtml(url);

    // Парсинг HTML
    GumboOutput* output = gumbo_parse(html.c_str());

    // Вектор для хранения ссылок
    std::vector<std::string> links;
    parseHtml(output->root, links);

    // Завершение работы с Gumbo
    gumbo_destroy_output(&kGumboDefaultOptions, output);

    // Создание нового Excel файла
    lxw_workbook* workbook = workbook_new("links.xlsx");
    lxw_worksheet* worksheet = workbook_add_worksheet(workbook, NULL);

    // Запись ссылок в Excel файл
    int row = 0;
    for (size_t i = 0; i < links.size(); ++i) {
        worksheet_write_number(worksheet, row, 0, i + 1, NULL);  // Нумерация
        worksheet_write_string(worksheet, row, 1, links[i].c_str(), NULL);  // Ссылки
        ++row;
    }

    // Закрытие Excel файла
    workbook_close(workbook);

    std::cout << "Links have been written to links.xlsx" << std::endl;

    return 0;
}
