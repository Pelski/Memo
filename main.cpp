#include <iostream>
#include <list>
#include <sstream>
#include <fstream>
#include "colors.h"
#include <sqlite3.h>
#include <sys/param.h>
#include "boost/date_time/posix_time/posix_time.hpp"

using namespace std;
using namespace boost::posix_time;

#define TASKS_FILE "/Users/pelski/.tasks.db"

enum Command {
    T_NONE,
    T_ADD,
    T_LAST,
    T_LIST,
    T_RM,
    T_NEW,
    T_PENDING,
    T_DONE,
    T_FIND,
    T_EDIT,
    T_I,
    T_II,
    T_III,
    T_IIII,
    T_FIVE,
    T_ACTUAL,
    T_RUN,
    T_EXIT,
    T_HELP,
    T_PROJECT,
    T_TAG,
    T_DESC
};

enum TaskPriority {
    NORMAL,
    HIGH,
    HIGHER,
    HIGHEST
};

enum TaskStatus {
    NONE,
    PENDING,
    DONE,
    ALL
};

struct Task {
    int ID = 0;
    string taskName = "";
    string date = "";
    int status = 0;
    int priority = 0;
    string project = "";
    string tags = "";
    string desc = "";

    ~Task() {
        ID = 0;
        taskName = "";
        date = "";
        status = 0;
        priority = 0;
        project = "";
        tags = "";
        desc = "";
    }
};

struct String {
    string text;
};

list<Task> tasks;
Task taskNew = {};
sqlite3 *db;

void showError(const string &err) {
    cerr << termcolor::bold << termcolor::red << " >> " << err << termcolor::reset << endl;
}

bool startsWithCaseInsensitive(string mainStr, string toMatch) {
    transform(mainStr.begin(), mainStr.end(), mainStr.begin(), ::tolower);
    transform(toMatch.begin(), toMatch.end(), toMatch.begin(), ::tolower);

    return mainStr.find(toMatch) == 0;
}

bool compareStrings(const string &s1, const string &s2) {
    return s1 == s2;
}

void displayTask(Task task) {
    string tmp = "#" + to_string(task.ID);
    cout.width(7);
    cout << termcolor::cyan;
    cout << std::right << tmp << " ";

    cout.width(10);
    cout << termcolor::green << task.date << " ";

    switch (task.priority) {
        case NORMAL: cout << termcolor::white << setw(4) << " "; break;
        case HIGH: cout << termcolor::blue << setw(4) << "! "; break;
        case HIGHER: cout << termcolor::yellow << setw(4) << "!! "; break;
        case HIGHEST: cout << termcolor::red << setw(4) << "!!! "; break;
        default:break;
    }

    cout << termcolor::reset << termcolor::bold;
    switch (task.status) {
        case NONE: cout << "[ ]"; break;
        case PENDING: cout << "[•]"; break;
        case DONE: cout << "[√]"; break;
        default:break;
    }

    cout << " ";

    switch (task.priority) {
        case NORMAL: cout << termcolor::white; break;
        case HIGH: cout << termcolor::blue; break;
        case HIGHER: cout << termcolor::yellow; break;
        case HIGHEST: cout << termcolor::red; break;
        default:break;
    }

    cout << std::left << task.taskName << termcolor::reset;
    if (task.project.length() > 0) {
        cout << "  " << termcolor::on_red << termcolor::white << " @" << task.project << " " << termcolor::reset;
    }

    if (task.tags.length() > 0) {
        cout << " " << termcolor::on_blue << termcolor::white << " " << task.tags << termcolor::reset;
    }
    cout << endl;

    if (task.desc.length() > 1) {
        cout.width(39);
        cout << termcolor::white << termcolor::on_grey << std::left << task.desc << termcolor::reset;
        cout << endl;
    }

}

void showTasks() {
    for (auto &task : tasks) {
        if (task.status != 2) {
            displayTask({task.ID, task.taskName, task.date, task.status, task.priority, task.project, task.tags, task.desc });
        }
    }
}

void showTasks(TaskStatus status) {
    for (auto &task : tasks) {
        if (task.status == status || status == ALL) {
            displayTask({task.ID, task.taskName, task.date, task.status, task.priority, task.project, task.tags, task.desc });
        }
    }
}

void showLastTasks(int x) {
    int i = 0;
    for (auto &task : tasks) {
        if (i >= x) {
            break;
        }
        displayTask({ task.ID, task.taskName, task.date, task.status, task.priority, task.project, task.tags, task.desc });
        i++;
    }
}

void showTaskFromProject(string project) {
    for (auto &task : tasks) {
        if (task.project == project) {
            displayTask({task.ID, task.taskName, task.date, task.status, task.priority, task.project, task.tags, task.desc });
        }
    }
}

void showTasksWithTag(string tag) {
    cout << "TAG: " << tag << endl;
    for (auto &task : tasks) {
        if (string::npos != task.tags.find(tag)) {
            displayTask({task.ID, task.taskName, task.date, task.status, task.priority, task.project, task.tags, task.desc });
        }
    }
}

void showTask(int id) {
    for (auto &task : tasks) {
        if (task.ID == id) {
            displayTask({task.ID, task.taskName, task.date, task.status, task.priority, task.project, task.tags, task.desc});
            return;
        }
    }

    showError("Nie znaleziono zadania o ID: " + to_string(id) + " do wyswietlenia!");
}

void removeTask(int id) {
    for (auto it = tasks.begin(); it != tasks.end(); it++) {
        if (it->ID == id) {
            tasks.erase(it);
            return;
        }
    }
    showError("Nie znaleziono zadania o ID: " + to_string(id) + " do usuniecia!");
}

void editTaskStatus(int id, TaskStatus taskStatus) {
    for (auto &task : tasks) {
        if (task.ID == id) {
            task.status = taskStatus;
            displayTask({task.ID, task.taskName, task.date, task.status, task.priority, task.project, task.tags});
            return;
        }
    }
    showError("Nie znaleziono zadania o ID: " + to_string(id) + " do edycji!");
}

void editTaskPriority(int id, TaskPriority taskPriority) {
    for (auto &task : tasks) {
        if (task.ID == id) {
            task.priority = taskPriority;
            displayTask({task.ID, task.taskName, task.date, task.status, task.priority, task.project, task.tags});
            return;
        }
    }
    showError("Nie znaleziono zadania o ID: " + to_string(id) + " do edycji!");
}

static int callback_write(void *NotUsed, int argc, char **argv, char **azColName) {
    return 0;
}

static int callback_read(void *data, int argc, char **argv, char **azColName){
    int i;
    Task task;
    for(i = 0; i<argc; i++){

        if (compareStrings(azColName[i], "id")) task.ID = stoi(argv[i]);
        if (compareStrings(azColName[i], "taskName")) task.taskName = argv[i];
        if (compareStrings(azColName[i], "status")) task.status = stoi(argv[i]);
        if (compareStrings(azColName[i], "tags")) task.tags = argv[i];
        if (compareStrings(azColName[i], "date")) task.date = argv[i];
        if (compareStrings(azColName[i], "project")) task.project = argv[i];
        if (compareStrings(azColName[i], "desc")) task.desc = argv[i];
        if (compareStrings(azColName[i], "priority")) task.priority = stoi(argv[i]);
    }
    tasks.push_back(task);

    return 0;
}

void loadFromFile() {
    char *zErrMsg = 0;
    int rc;

    rc = sqlite3_open(TASKS_FILE, &db);

    if (rc) {
        showError("Can't open database: " + (string)sqlite3_errmsg(db));
        exit(0);
    } else {
        const char *sql = "CREATE TABLE tasks("  \
         "id INT PRIMARY KEY NOT NULL," \
         "taskName TEXT NOT NULL," \
         "status INT NOT NULL," \
         "tags CHAR(150) NOT NULL," \
         "date CHAR(50) NOT NULL," \
         "project CHAR(50) NOT NULL," \
         "desc TEXT," \
         "priority INT NOT NULL);";

        rc = sqlite3_exec(db, sql, callback_write, 0, &zErrMsg);

        if (rc != SQLITE_OK) {
            sqlite3_free(zErrMsg);
        }

        sql = "SELECT * FROM tasks";
        const char* data = "Callback function called";
        rc = sqlite3_exec(db, sql, callback_read, (void*)data, &zErrMsg);

        if (rc != SQLITE_OK) {
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
        }
    }
    sqlite3_close(db);
}

void saveToFile() {
    sqlite3 *db;
    char *zErrMsg = 0;
    int rc;
    string sql;

    rc = sqlite3_open(TASKS_FILE, &db);

    if (rc) {
        showError("Can't open database: " + (string)sqlite3_errmsg(db));
        exit(0);
    }

    sql = "DELETE FROM tasks";
    rc = sqlite3_exec(db, sql.c_str(), callback_write, 0, &zErrMsg);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }

    sql = "";
    for (auto &it : tasks) {
        sql += "INSERT INTO tasks (id, taskName, status, tags, date, project, desc, priority) " \
          "VALUES(" + to_string(it.ID) + ", " \
          "\"" + it.taskName + "\", " \
          "" + to_string(it.status) + ", " \
          "\"" + it.tags + "\", " \
          "\"" + it.date + "\", " \
          "\"" + it.project + "\"," \
          "\"" + it.desc + "\"," \
          "" + to_string(it.priority) + ");";
    }

    rc = sqlite3_exec(db, sql.c_str(), callback_write, 0, &zErrMsg);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    sqlite3_close(db);
}

void help() {
    cout << termcolor::bold << termcolor::red << "Usage:" << termcolor::white << " memo param" << endl << endl;
    cout << termcolor::bold << "Jak uzywac memo?" << termcolor::reset << endl;
    cout << termcolor::bold << " memo" << termcolor::reset << " add/a xxxxx - dodaj zadanie" << endl;
    cout << "   --i/$0 - zadanie z normalnym priorytetem (dodawane automatycznie/wymagane przy edycji)" << endl;
    cout << "   --ii/$1 - zadanie z wysokim priorytetem" << endl;
    cout << "   --iii/$2 - zadanie z wyzszym priorytetem" << endl;
    cout << "   --iiii/$3 - zadanie z najwyzszym priorytetem" << endl;
    cout << "   --normal - zadanie do zrobienia" << endl;
    cout << "   --done - zadanie wykonane" << endl;
    cout << "   --pending - zadanie w trakcie wykonywania" << endl;
    cout << termcolor::bold << " memo" << termcolor::reset << " last/l xxxx - wyswietlenie xxxx ostatnich zadan" << endl;
    cout << termcolor::bold << " memo" << termcolor::reset << " project/pr yyyy - wyswietlenie zadan z listy" << endl;
    cout << termcolor::bold << " memo" << termcolor::reset << " list/ls - wyswietlanie zadan" << endl;
    cout << termcolor::bold << " memo" << termcolor::reset << " list/ls all/a - wyswietlanie zadan (z ukonczonymi)" << endl;
    cout << termcolor::bold << " memo" << termcolor::reset << " list/ls done/d - wyswietlanie zadan ukonczonych" << endl;
    cout << termcolor::bold << " memo" << termcolor::reset << " list/ls pending/p - wyswietlanie zadan w trakcie wykonywania" << endl;
    cout << termcolor::bold << " Wyswietlanie (list) moze miec rowniez dodatkowe parametry:" << termcolor::reset << endl;
    cout << "   project/pro - sortowanie wedlug nazwy projektu" << endl;
    cout << "   projectr/pror - odwrocone sortowanie wedlug nazwy projektu" << endl;
    cout << "   priority/pri - sortowanie wedlug piorytetu" << endl;
    cout << "   priorityr/prir - odwrocone sortowanie wedlug piorytetu" << endl;
    cout << "   id/i - sortowanie wedlug ID" << endl;
    cout << "   idr/ir - odwrocone sortowanie wedlug ID" << endl;
    cout << "   status/s - sortowanie wedlug statusu" << endl;
    cout << "   statusr/sr - odwrocone sortowanie wedlug statusu" << endl;
    cout << termcolor::bold << " memo" << termcolor::reset << " find/s xxxx - wyswietlenie zadania o podanym ID" << endl;
    cout << termcolor::bold << " memo" << termcolor::reset << " desc/ds xxxx yyyy - zmiana opisu dla zadania o podanym ID" << endl;
    cout << termcolor::bold << " memo" << termcolor::reset << " tag/t xxxx - wyswietlenie zadan z tagiem" << endl;
    cout << termcolor::bold << " memo" << termcolor::reset << " rm yy - usuwanie zadania o ID yy" << endl;
    cout << termcolor::bold << " memo" << termcolor::reset << " new/n yy - ustawianie statusu zadania na nowe" << endl;
    cout << termcolor::bold << " memo" << termcolor::reset << " pending/p yy - ustawianie statusu zadania na w trakcie wykonywania" << endl;
    cout << termcolor::bold << " memo" << termcolor::reset << " done/d yy - ustawianie statusu zadania na wykonane" << endl;
    cout << termcolor::bold << " memo" << termcolor::reset << " edit/e yy - tryb edycji zadania" << endl;
    cout << termcolor::bold << " memo" << termcolor::reset << " i yy - zmiana priorytetu zadania o ID na normalny" << endl;
    cout << termcolor::bold << " memo" << termcolor::reset << " ii yy - zmiana priorytetu zadania o ID na wysoki" << endl;
    cout << termcolor::bold << " memo" << termcolor::reset << " iii yy - zmiana priorytetu zadania o ID na wyzszy" << endl;
    cout << termcolor::bold << " memo" << termcolor::reset << " iiii yy - zmiana priorytetu zadania o ID na najwyzszy" << endl;
    cout << termcolor::bold << " memo" << termcolor::reset << " five/f - ostatnie piec zadan" << endl;
    cout << termcolor::bold << " memo" << termcolor::reset << " actual/todo - aktualne zadania o statusie w trakcie wykonywania" << endl;
    cout << termcolor::bold << " memo" << termcolor::reset << " help - pomoc" << endl;
    cout << termcolor::bold << " memo" << termcolor::reset << " run - uruchomienie trybu interaktywnego" << endl;
    cout << termcolor::bold << " memo" << termcolor::reset << " exit - wyjscie z trybu interaktywnego" << endl;
    cout << endl;
    cout << "      @projekt  ^tag" << endl;
}

int main(int argc, char** argv) {
    if (argc <= 1) {
        argc++;
        argv[1] = const_cast<char *>("ls");
    }

    char username[MAXLOGNAME];
    getlogin_r(username, MAXLOGNAME);

    bool appExit = true;
    do {
        // Loading database
        loadFromFile();

        // Parsing arguments to list of words
        list<String> words;
        if (compareStrings(argv[1], "run")) {
            appExit = false;
            words.clear();

            char *wordsInput = new char[1024];
            cout << "<" << username << "@Memo>: ";
            cin.clear();
            cin.getline(wordsInput, 1024, '\n');

            stringstream ssin(wordsInput);
            while (ssin.good()) {
                string word;
                ssin >> word;
                words.push_back({ word });
            }
        } else {
            for (int i = 1; i < argc; i++) {
                words.push_back({ argv[i] });
            }
        }

        // For single arg commands
        words.push_back({ "" });
        cout << endl;

        Command command = T_NONE;
        int wordNumber = -1;

        // Sorting tasks
        tasks.sort([]( const Task& a, const Task&b ) {
            return a.ID > b.ID;
        });

        taskNew.priority = NORMAL;
        taskNew.project = "";
        taskNew.taskName = "";
        taskNew.ID = tasks.front().ID + 1;
        taskNew.status = NONE;
        taskNew.tags = "";
        taskNew.desc = "";

        string listSortType = "";

        bool isReady = false;

        for (auto &word : words) {
            wordNumber++;
            if (wordNumber == 0) {
                //cout << " -- Command: " << it->text << endl;
                if (compareStrings(word.text, "add") || compareStrings(word.text, "a")) command = T_ADD;
                else if (compareStrings(word.text, "last") || compareStrings(word.text, "l")) command = T_LAST;
                else if (compareStrings(word.text, "list") || compareStrings(word.text, "ls")) command = T_LIST;
                else if (compareStrings(word.text, "find") || compareStrings(word.text, "s")) command = T_FIND;
                else if (compareStrings(word.text, "desc") || compareStrings(word.text, "ds")) command = T_DESC;
                else if (compareStrings(word.text, "tag") || compareStrings(word.text, "t")) command = T_TAG;
                else if (compareStrings(word.text, "rm")) command = T_RM;
                else if (compareStrings(word.text, "new") || compareStrings(word.text, "n")) command = T_NEW;
                else if (compareStrings(word.text, "pending") || compareStrings(word.text, "p")) command = T_PENDING;
                else if (compareStrings(word.text, "done") || compareStrings(word.text, "d")) command = T_DONE;
                else if (compareStrings(word.text, "edit") || compareStrings(word.text, "e")) command = T_EDIT;
                else if (compareStrings(word.text, "project") || compareStrings(word.text, "pr")) command = T_PROJECT;
                else if (compareStrings(word.text, "i")) command = T_I;
                else if (compareStrings(word.text, "ii")) command = T_II;
                else if (compareStrings(word.text, "iii")) command = T_III;
                else if (compareStrings(word.text, "iiii")) command = T_IIII;
                else if (compareStrings(word.text, "five") || compareStrings(word.text, "f")) command = T_FIVE;
                else if (compareStrings(word.text, "actual") || compareStrings(word.text, "todo"))command = T_ACTUAL;
                else if (compareStrings(word.text, "run")) command = T_RUN;
                else if (compareStrings(word.text, "exit")) command = T_EXIT;
                else if (compareStrings(word.text, "help")) command = T_HELP;
                else if (compareStrings(word.text, "clear") || compareStrings(word.text, "cls")) system("clear");
                else { showError("Nie znaleziono podanego polecenia. Mozesz uzyc komendy 'help' aby uzyskac pomoc!"); break; }
            } else {
                if (command == T_ADD || command == T_EDIT) {
                    if (command == T_EDIT) {
                        // ID zadania
                        if (wordNumber == 1) {
                            if (word.text.length() > 0) {
                                int id = 0;
                                try {
                                    id = stoi(word.text);
                                } catch (exception &ex) {
                                    continue;
                                }
                                bool found = false;
                                for (auto &task : tasks) {
                                    if (task.ID == id) {
                                        taskNew.ID = task.ID;
                                        taskNew.taskName = "";
                                        taskNew.date = task.date;
                                        taskNew.status = task.status;
                                        taskNew.priority = task.priority;
                                        taskNew.project = task.project;
                                        taskNew.tags = "";
                                        taskNew.desc = "";
                                        found = true;
                                        isReady = true;
                                        break;
                                    }
                                }

                                if (!found) {
                                    showError("Nie znaleziono zadania o podanym ID!");
                                }
                            } else {
                                showError("Nie znaleziono ID zadania!");
                                break;
                            }
                            continue;
                        }
                    }

                    if (startsWithCaseInsensitive(word.text, "@")) {
                        taskNew.project = word.text.substr(1, word.text.length() - 1);
                    } else if (compareStrings(word.text, "--iiii") || compareStrings(word.text, "$3")) {
                        taskNew.priority = HIGHEST;
                    } else if (compareStrings(word.text, "--iii") || compareStrings(word.text, "$2")) {
                        taskNew.priority = HIGHER;
                    } else if (compareStrings(word.text, "--ii") || compareStrings(word.text, "$1")) {
                        taskNew.priority = HIGH;
                    } else if (compareStrings(word.text, "--i") || compareStrings(word.text, "$0")) {
                        taskNew.priority = NORMAL;
                    } else if (compareStrings(word.text, "--normal")) {
                        taskNew.status = NONE;
                    } else if (compareStrings(word.text, "--pending")) {
                        taskNew.status = PENDING;
                    } else if (compareStrings(word.text, "--done")) {
                        taskNew.status = DONE;
                    } else if (startsWithCaseInsensitive(word.text, "^")) {
                        taskNew.tags += word.text.substr(1, word.text.length() - 1) + " ";
                    } else {
                        taskNew.taskName += word.text + " ";
                        isReady = true;
                    }
                }

                if (command == T_LAST) {
                    int i = 1;
                    if (word.text.length() > 0) {
                        i = stoi(word.text);
                    }

                    showLastTasks(i);
                    break;
                }

                if (command == T_DESC) {
                    if (wordNumber == 1) {
                        if (word.text.length() > 0) {
                            try {
                                taskNew.ID = stoi(word.text);
                            } catch (exception ex) {
                                showError("Musisz podac ID!");
                                command = T_NONE;
                                break;
                            }
                        }
                    } else {
                        taskNew.desc += " " + word.text;
                    }
                }

                if (command == T_FIND) {
                    if (word.text.length() > 0) {
                        showTask(stoi(word.text));
                    } else {
                        showError("Nie znaleziono ID zadania!");
                    }
                    break;
                }

                if (command == T_PROJECT) {
                    if (word.text.length() > 0) {
                        showTaskFromProject(word.text);
                    } else {
                        showError("Nie znaleziono projektu!");
                    }
                    break;
                }

                if (command == T_RM) {
                    if (word.text.length() > 0) {
                        removeTask(stoi(word.text));
                    } else {
                        showError("Nie znaleziono ID zadania!");
                    }
                    break;
                }

                if (command == T_NEW) {
                    if (word.text.length() > 0) {
                        editTaskStatus(stoi(word.text), NONE);
                    } else {
                        showError("Nie znaleziono ID zadania!");
                    }
                    break;
                }

                if (command == T_PENDING) {
                    if (word.text.length() > 0) {
                        editTaskStatus(stoi(word.text), PENDING);
                    } else {
                        showError("Nie znaleziono ID zadania!");
                    }
                    break;
                }

                if (command == T_DONE) {
                    if (word.text.length() > 0) {
                        editTaskStatus(stoi(word.text), DONE);
                    } else {
                        showError("Nie znaleziono ID zadania!");
                    }
                    break;
                }

                if (command == T_FIVE) {
                    showLastTasks(5);
                    break;
                }

                if (command == T_LIST) {
                    if (wordNumber == 1) {
                        if (compareStrings(word.text, "status") || compareStrings(word.text, "") || compareStrings(word.text, " ")) {
                            tasks.sort([]( const Task& a, const Task&b ) {
                                return a.status > b.status;
                            });
                        }
                    }

                    if (compareStrings(word.text, "statusr") || compareStrings(word.text, "sr")) {
                        tasks.sort([]( const Task& a, const Task&b ) {
                            return a.status > b.status;
                        });
                    }

                    if (compareStrings(word.text, "priority") || compareStrings(word.text, "pri")) {
                        tasks.sort([]( const Task& a, const Task&b ) {
                            return a.priority > b.priority;
                        });
                    }
                    if (compareStrings(word.text, "priorityr") || compareStrings(word.text, "prir")) {
                        tasks.sort([]( const Task& a, const Task&b ) {
                            return a.priority < b.priority;
                        });
                    }

                    if (compareStrings(word.text, "id") || compareStrings(word.text, "i")) {
                        tasks.sort([]( const Task& a, const Task&b ) {
                            return a.ID > b.ID;
                        });
                    }

                    if (compareStrings(word.text, "idr") || compareStrings(word.text, "ir")) {
                        tasks.sort([]( const Task& a, const Task&b ) {
                            return a.ID < b.ID;
                        });
                    }

                    if (compareStrings(word.text, "project") || compareStrings(word.text, "pro")) {
                        tasks.sort([]( const Task& a, const Task&b ) {
                            return a.project > b.project;
                        });
                    }

                    if (compareStrings(word.text, "projectr") || compareStrings(word.text, "pror")) {
                        tasks.sort([]( const Task& a, const Task&b ) {
                            return a.project < b.project;
                        });
                    }

                    if (compareStrings(word.text, "pending") || compareStrings(word.text, "p")) listSortType = word.text;
                    else if (compareStrings(word.text, "done") || compareStrings(word.text, "d")) listSortType = word.text;
                    else if (compareStrings(word.text, "all") || compareStrings(word.text, "a")) listSortType = word.text;
                }

                if (command == T_I || command == T_II || command == T_III || command == T_IIII) {
                    if (word.text.length() > 0) {
                        int id = stoi(word.text);
                        if (command == T_I) editTaskPriority(id, NORMAL);
                        if (command == T_II) editTaskPriority(id, HIGH);
                        if (command == T_III) editTaskPriority(id, HIGHER);
                        if (command == T_IIII) editTaskPriority(id, HIGHEST);
                    }
                }

                if (command == T_TAG) {
                    showTasksWithTag(word.text);
                    break;
                }

                if (command == T_ACTUAL) {
                    showTasks(PENDING);
                    break;
                }
            }
        }

        // Commit
        if (command == T_ADD && isReady) {
            time_facet *facet = new time_facet("%H:%M %d/%m/%Y");
            ostringstream oss;
            oss.imbue(locale(oss.getloc(), facet));
            oss << second_clock::local_time();
            taskNew.date = oss.str();
            tasks.push_front(taskNew);
            cout << termcolor::cyan << termcolor::bold << "#" << taskNew.ID << termcolor::reset << " created" << endl;
        }

        if (command == T_EDIT && isReady) {
            //removeTask(taskNew.ID);
            if (taskNew.taskName.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890_-()[].,<>?{}'\"\\%*@/^&$#!;:| ") != std::string::npos) {
                std::cerr << "Unallowed characters, try again...\n";
            } else {
                bool founded = false;
                for (auto it = tasks.begin(); it != tasks.end(); it++) {
                    if (it->ID == taskNew.ID) {
                        it->taskName = taskNew.taskName;
                        it->status = taskNew.status;
                        it->project = taskNew.project;
                        it->priority = taskNew.priority;
                        it->date = taskNew.date;
                        it->tags = taskNew.tags;
                        it->desc = taskNew.desc;
                        founded = true;
                        break;
                    }
                }
                if (founded) {
                    cout << termcolor::cyan << termcolor::bold << "#" << taskNew.ID << termcolor::reset << " edited" << endl;
                    showTask(taskNew.ID);
                } else {
                    showError("Nie znaleziono zadania o podanym ID!");
                }
            }
        }

        if (command == T_DESC) {
            if (taskNew.taskName.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890_-()[].,<>?{}'\"\\%*@/^&$#!;:| ") != std::string::npos) {
                std::cerr << "Unallowed characters, try again...\n";
            } else {
                bool founded = false;
                for (auto it = tasks.begin(); it != tasks.end(); it++) {
                    if (it->ID == taskNew.ID) {
                        it->desc = taskNew.desc;
                        founded = true;
                        break;
                    }
                }
                if (founded) {
                    cout << termcolor::cyan << termcolor::bold << "#" << taskNew.ID << termcolor::reset << " description edited" << endl;
                    showTask(taskNew.ID);
                } else {
                    showError("Nie znaleziono zadania o podanym ID!");
                }
            }
        }

        if (command == T_LIST) {
            if (compareStrings(listSortType, "pending") || compareStrings(listSortType, "p")) showTasks(PENDING);
            else if (compareStrings(listSortType, "done") || compareStrings(listSortType, "d")) showTasks(DONE);
            else if (compareStrings(listSortType, "all") || compareStrings(listSortType, "a")) showTasks(ALL);
            else showTasks();
        }

        if (command == T_RUN) {
            appExit = false;
        }

        if (command == T_EXIT) {
            appExit = true;
        }

        if (command == T_HELP) {
            help();
        }

        cout << endl;

        saveToFile();
        tasks.clear();
    } while (!appExit);

    return 0;
}