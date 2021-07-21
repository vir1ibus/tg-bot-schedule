#include <string>
#include <sqlite3.h>
#include <ctime>

#include <tgbot/tgbot.h>

using namespace std;
using namespace TgBot;

struct Task {
    int id;
    string name;
    string description;
    int task_list_id;
    string date_creation;
    string date_reminder_start;
    string date_reminder_end;
};

struct Task_list {
    int id;
    string name;
    int32_t owner;
};

class Database {
private:
    int rc = 0;
    sqlite3 *db;
    char *name_db;
    const char* sql;
    char *zErrMsg = 0;
    void check_success(int rc){
        if( rc != SQLITE_OK ){
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
        }
    }

public:
    Database(){
    rc = sqlite3_open("app.db", &db);
    if( rc ) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return;
    } else {
        sql = "CREATE TABLE IF NOT EXISTS task_list( " \
        "id int(11) PRIMARY KEY, " \
        "name varchar(255) NOT NULL, " \
        "owner int(11) NOT NULL);";
        
        rc = sqlite3_exec(db, sql, [](void* data, int argc, char** argv, char** azColName){ return 0; }, 0, &zErrMsg);

        check_success(rc);

        sql = "CREATE TABLE IF NOT EXISTS task (" \
        "id int(11) NOT NULL," \
        "name varchar(255) NOT NULL," \
        "description longtext DEFAULT NULL," \
        "task_list_id int(11) NOT NULL, " \ 
        "date_creation datetime NOT NULL DEFAULT current_timestamp," \
        "date_reminder_start datetime DEFAULT NULL," \
        "date_reminder_end datetime DEFAULT NULL," \
        "PRIMARY KEY (id), " \
        "FOREIGN KEY (task_list_id) REFERENCES task_list(id) );";
        
        rc = sqlite3_exec(db, sql, [](void* data, int argc, char** argv, char** azColName){ return 0; }, 0, &zErrMsg);

        check_success(rc);
    }
}

vector<Task_list> get_task_lists(int32_t id){
    vector<Task_list> task_lists;
    string sql = "SELECT * FROM task_list WHERE task_list.owner = " + to_string(id) + ";";
    sqlite3_stmt *stmt;

    rc = sqlite3_prepare(db, sql.c_str(), -1, &stmt, NULL);

    while(sqlite3_step(stmt) == SQLITE_ROW){
        Task_list tmp;
        tmp.id = sqlite3_column_int(stmt, 0);
        tmp.name = (char*)sqlite3_column_text(stmt, 1);
        tmp.owner = sqlite3_column_int(stmt, 2);
        task_lists.push_back(tmp);
    }

    sqlite3_finalize(stmt);
    return task_lists;
}

vector<Task> get_task(int32_t task_list_id){
    vector<Task> tasks;
    string sql = "SELECT * FROM task WHERE task.task_list_id = " + to_string(task_list_id) + ";";
    sqlite3_stmt *stmt;

    rc = sqlite3_prepare(db, sql.c_str(), -1, &stmt, NULL);

    while(sqlite3_step(stmt) == SQLITE_ROW){
        Task tmp;
        tmp.id = sqlite3_column_int(stmt, 0);
        tmp.name = (char*)sqlite3_column_text(stmt, 1);
        tmp.description = (char*)sqlite3_column_text(stmt, 2);
        tmp.date_creation = (char*)sqlite3_column_text(stmt, 3);
        tmp.date_reminder_start = (char*)sqlite3_column_text(stmt, 4);
        tmp.date_reminder_end = (char*)sqlite3_column_text(stmt, 5);
        tasks.push_back(tmp);
    }

    sqlite3_finalize(stmt);
    return tasks;
}


int get_last_id_table(string name){
    int id = 0;
    string sql = "SELECT max(id) FROM " + name + ";";
    sqlite3_stmt *stmt;

    rc = sqlite3_prepare(db, sql.c_str(), -1, &stmt, NULL);

    check_success(rc);

    while(sqlite3_step(stmt) == SQLITE_ROW){
        id = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
    return id;
}

int32_t get_owner_task_list(int task_list_id){
    int32_t id = 0;
    string sql = "SELECT owner FROM task_list WHERE task_list.id = " + to_string(task_list_id) + ";";
    cout << sql << endl;
    sqlite3_stmt *stmt;

    rc = sqlite3_prepare(db, sql.c_str(), -1, &stmt, NULL);

    check_success(rc);

    while(sqlite3_step(stmt) == SQLITE_ROW){
        cout << sqlite3_column_int(stmt, 0) << endl;
        id = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
    
    return id;
}

void create_task_list(int32_t id, string name){
    string sql = "INSERT INTO task_list VALUES(" + to_string(get_last_id_table("task_list")+1) + ", '" + name + "'," + to_string(id) + ");";
    rc = sqlite3_exec(db, sql.c_str(), NULL, NULL, &zErrMsg);
    check_success(rc);
}

void create_task(int task_list_id, string name, string description, string date_creation, string date_reminder_start, string date_reminder_end){
    string sql = "INSERT INTO task_list(id, name, description, date_reminder_start, date_reminder_end) VALUES(" + to_string(get_last_id_table("task") + 1) + ", '" + name + "', '" + description + "', '" + date_reminder_start + "', '" + date_reminder_end + "');"; 
    rc = sqlite3_exec(db, sql.c_str(), NULL, NULL, &zErrMsg);
    check_success(rc);
}

};

void show_task_list(Message::Ptr message, Bot bot, InlineKeyboardMarkup::Ptr keyboard, vector<InlineKeyboardButton::Ptr> row, Database *database){
    vector<Task_list> task_lists = database -> get_task_lists(message->from->id);
    string text;
    InlineKeyboardButton::Ptr createBtn(new InlineKeyboardButton);
    createBtn->text = "Создать";
    createBtn->callbackData = "create_task_list";
    row.push_back(createBtn);
    if(task_lists.size() == 0){
        text = "Ваш список листов задач пуст.";
    } else {
        int j = 1;
        for(Task_list i : task_lists){
            text += to_string(j) + " " + i.name + " " + to_string(i.owner) + "\n";
            string data = "jump " + to_string(i.id);
            InlineKeyboardButton::Ptr button(new InlineKeyboardButton);
            button->text = to_string(j);
            button->callbackData = data;
            j++;
            row.push_back(button);
        }
    }

    keyboard->inlineKeyboard.push_back(row);
    bot.getApi().sendMessage(message->chat->id, text, false, 0, keyboard);
    row.clear();
    keyboard->inlineKeyboard.clear();
    task_lists.clear();
}

vector<Task_list> show_task_list(Message::Ptr message, int32_t owner, Bot bot, InlineKeyboardMarkup::Ptr keyboard, vector<InlineKeyboardButton::Ptr> row, Database *database){
    vector<Task_list> task_lists = database -> get_task_lists(owner);
    string text;
    InlineKeyboardButton::Ptr createBtn(new InlineKeyboardButton);
    createBtn->text = "Создать";
    createBtn->callbackData = "create_task_list";
    row.push_back(createBtn);
    if(task_lists.size() == 0){
        text = "Ваш список листов задач пуст.";
    } else {
        int j = 1;
        for(Task_list i : task_lists){
            text += to_string(j) + " " + i.name + " " + to_string(i.owner) + "\n";
            string data = "jump " + to_string(i.id);
            InlineKeyboardButton::Ptr button(new InlineKeyboardButton);
            button->text = to_string(j);
            button->callbackData = data;
            j++;
            row.push_back(button);
        }
    }

    keyboard->inlineKeyboard.push_back(row);
    bot.getApi().sendMessage(message->chat->id, text, false, 0, keyboard);
    row.clear();
    keyboard->inlineKeyboard.clear();
    task_lists.clear();
}

vector<Task> show_task(Message::Ptr message, Bot bot, InlineKeyboardMarkup::Ptr keyboard, vector<InlineKeyboardButton::Ptr> row, Database *database, int task_list_id){
    vector<Task> tasks = database -> get_task(task_list_id);
    string text;
    InlineKeyboardButton::Ptr createBtn(new InlineKeyboardButton);
    createBtn->text = "Создать";
    createBtn->callbackData = "create_task";
    row.push_back(createBtn);
    if(tasks.size() == 0){
        text = "Ваш список задач пуст.";
    } else {
        int j = 1;
        for(Task i : tasks){
            text += to_string(j) + " " + i.name + " " + i.description + " " + i.date_creation + " " + i.date_reminder_start + " " + i.date_reminder_end + "\n";
            string data = "jump " + to_string(i.id);
            InlineKeyboardButton::Ptr button(new InlineKeyboardButton);
            button->text = to_string(j);
            button->callbackData = data;
            j++;
            row.push_back(button);
        }
    }

    InlineKeyboardButton::Ptr backBtn(new InlineKeyboardButton);
    string data = "back_to_task_list " + to_string(task_list_id);
    backBtn->text = "Назад";
    backBtn->callbackData = data;
    row.push_back(backBtn);

    keyboard->inlineKeyboard.push_back(row);
    bot.getApi().sendMessage(message->chat->id, text, false, 0, keyboard);
    row.clear();
    keyboard->inlineKeyboard.clear();
    tasks.clear();
}



int main() {
    const int CHECK_TASK_LIST_STATE = 0;
    const int CHECK_TASK_STATE = 1;
    const int CREATE_TASK_LIST_STATE = 2;
    const int CREATE_TASK_STATE = 3;

    int prev_state = CHECK_TASK_LIST_STATE;
    int state = CHECK_TASK_LIST_STATE;

    Database *database = new Database();
    Bot bot("1620046312:AAEbb89mVnnnO1KvPwA2YY0hf9lRGQbgg-Q");
    InlineKeyboardMarkup::Ptr keyboard(new InlineKeyboardMarkup);
    vector<InlineKeyboardButton::Ptr> row;

    bot.getEvents().onCommand("start", [&bot, &keyboard, &row, &database](Message::Ptr message) {
        show_task_list(message, bot, keyboard, row, database);
    });

    bot.getEvents().onCallbackQuery([&bot, &keyboard, &database, &row, &state](CallbackQuery::Ptr query) {
        vector<Task_list> tmp_list;
        vector<Task> tmp_tasks;
        if (StringTools::startsWith(query->data, "create_task_list")) {
            state = CREATE_TASK_LIST_STATE;
            bot.getApi().sendMessage(query->message->chat->id, "Введите название списка.", false, 0);
        }
        if (StringTools::startsWith(query->data, "jump")) {
            state = CHECK_TASK_STATE;
            int task_list_id = atoi(query->data.substr(5, sizeof(query->data)-5).c_str());
            tmp_tasks = show_task(query->message, bot, keyboard, row, database, task_list_id);
            state = CHECK_TASK_LIST_STATE;
        }
        if (StringTools::startsWith(query->data, "back_to_task_list")) {
            state = CHECK_TASK_LIST_STATE;
            int task_list_id = atoi(query->data.substr(17, sizeof(query->data)-17).c_str());
            show_task_list(query->message, database->get_owner_task_list(task_list_id), bot, keyboard, row, database);
        }
    });

    bot.getEvents().onAnyMessage([&bot, &keyboard, &database, &row, &state, &prev_state](Message::Ptr message) {
        switch(state){
            case CREATE_TASK_LIST_STATE:
                database -> create_task_list(message->from->id, message->text.c_str());
                state = CHECK_TASK_LIST_STATE;
                show_task_list(message, bot, keyboard, row, database);
            break;
        }
    });

    try {
        printf("Bot username: %s\n", bot.getApi().getMe()->username.c_str());
        TgLongPoll longPoll(bot);
        while (true) {
            printf("Long poll started\n ");
            longPoll.start();
        }
    } catch (TgException& e) {
        printf("error: %s\n", e.what());
    }

    return 0;
}
