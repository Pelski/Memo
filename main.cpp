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
    T_PROJECT
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

    ~Task() {
        ID = 0;
        taskName = "";
        date = "";
        status = 0;
        priority = 0;
        project = "";
        tags = "";
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
    cout << termcolor::green << task.date << " -- ";
    cout << termcolor::bold;

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

//    cout.width(45);
    cout << std::left << task.taskName << termcolor::reset;
    if (task.project.length() > 0) {
        cout << termcolor::bold << termcolor::magenta << "   @" << task.project << " " << termcolor::reset;
    }

    if (task.tags.length() > 0) {
        cout << termcolor::bold << termcolor::yellow << task.tags << termcolor::reset;
    }
    cout << endl;
}

void showTasks() {
    for (auto &task : tasks) {
        if (task.status != 2) {
            displayTask({task.ID, task.taskName, task.date, task.status, task.priority, task.project, task.tags });
        }
    }
}

void showTasks(TaskStatus status) {
    for (auto &task : tasks) {
        if (task.status == status || status == ALL) {
            displayTask({task.ID, task.taskName, task.date, task.status, task.priority, task.project, task.tags });
        }
    }
}

void showLastTasks(int x) {
    int i = 0;
    for (auto &task : tasks) {
        if (i >= x) {
            break;
        }
        displayTask({task.ID, task.taskName, task.date, task.status, task.priority, task.project, task.tags });
        i++;
    }
}

void showTaskFromProject(string project) {
    for (auto &task : tasks) {
        if (task.project == project) {
            displayTask({task.ID, task.taskName, task.date, task.status, task.priority, task.project, task.tags});
        }
    }
}

void showTask(int id) {
    for (auto &task : tasks) {
        if (task.ID == id) {
            displayTask({task.ID, task.taskName, task.date, task.status, task.priority, task.project, task.tags});
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
    int i;
    for(i = 0; i<argc; i++) {
        //printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    //printf("\n");
    return 0;
}

static int callback_read(void *data, int argc, char **argv, char **azColName){
    int i;
    //fprintf(stderr, "%s: ", (const char*)data);

    Task task;
    for(i = 0; i<argc; i++){
        //printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
        if (compareStrings(azColName[i], "id")) task.ID = stoi(argv[i]);
        if (compareStrings(azColName[i], "taskName")) task.taskName = argv[i];
        if (compareStrings(azColName[i], "status")) task.status = stoi(argv[i]);
        if (compareStrings(azColName[i], "tags")) task.tags = argv[i];
        if (compareStrings(azColName[i], "date")) task.date = argv[i];
        if (compareStrings(azColName[i], "project")) task.project = argv[i];
        if (compareStrings(azColName[i], "priority")) task.priority = stoi(argv[i]);
    }
    tasks.push_back(task);

    //printf("\n");
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
        //printf(stderr, "Opened database successfully\n");
        const char *sql = "CREATE TABLE tasks("  \
         "id INT PRIMARY KEY NOT NULL," \
         "taskName TEXT NOT NULL," \
         "status INT NOT NULL," \
         "tags CHAR(150) NOT NULL," \
         "date CHAR(50) NOT NULL," \
         "project CHAR(50) NOT NULL," \
         "priority INT NOT NULL);";

        rc = sqlite3_exec(db, sql, callback_write, 0, &zErrMsg);

        if( rc != SQLITE_OK ){
            //showError("Can't open database: " + (string)sqlite3_errmsg(db));
            sqlite3_free(zErrMsg);
        } else {
            //fprintf(stdout, "Table created successfully\n");
        }

        /* Create SQL statement */
        sql = "SELECT * FROM tasks";

        /* Execute SQL statement */
        const char* data = "Callback function called";
        rc = sqlite3_exec(db, sql, callback_read, (void*)data, &zErrMsg);

        if( rc != SQLITE_OK ) {
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
        } else {
            //fprintf(stdout, "Operation done successfully\n");
        }
    }
    sqlite3_close(db);
}

void saveToFile() {
    sqlite3 *db;
    char *zErrMsg = 0;
    int rc;
    string sql = "";

    /* Open database */
    rc = sqlite3_open(TASKS_FILE, &db);

    if( rc ) {
        showError("Can't open database: " + (string)sqlite3_errmsg(db));
        exit(0);
    } else {
        //fprintf(stderr, "Opened database successfully\n");
    }

    sql = "DELETE FROM tasks";
    rc = sqlite3_exec(db, sql.c_str(), callback_write, 0, &zErrMsg);

    if( rc != SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    } else {
        //fprintf(stdout, "Records created successfully\n");
    }

    /* Create SQL statement */
    sql = "";
    for (auto &it : tasks) {
        sql += "INSERT INTO tasks (id, taskName, status, tags, date, project, priority) " \
          "VALUES(" + to_string(it.ID) + ", " \
          "\"" + it.taskName + "\", " \
          "" + to_string(it.status) + ", " \
          "\"" + it.tags + "\", " \
          "\"" + it.date + "\", " \
          "\"" + it.project + "\"," \
          "" + to_string(it.priority) + ");";
    }

    //cout << sql << endl;

    /* Execute SQL statement */
    rc = sqlite3_exec(db, sql.c_str(), callback_write, 0, &zErrMsg);

    if( rc != SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    } else {
        ///fprintf(stdout, "Records created successfully\n");
    }
    sqlite3_close(db);
}

void help() {
    cout << termcolor::bold << termcolor::red << "Usage:" << termcolor::white << " memo param" << endl << endl;
    cout << termcolor::bold << "Jak uzywac memo?" << termcolor::reset << endl;
    cout << " memo add xxxxx - dodaj zadanie" << endl;
    cout << "   --i - zadanie z normalnym priorytetem" << endl;
    cout << "   --ii - zadanie z wysokim priorytetem" << endl;
    cout << "   --iii - zadanie z wyzszym priorytetem" << endl;
    cout << "   --iiii - zadanie z najwyzszym priorytetem" << endl;
    cout << "   --normal - zadanie do zrobienia" << endl;
    cout << "   --done - zadanie wykonane" << endl;
    cout << "   --pending - zadanie w trakcie wykonywania" << endl;
    cout << " memo last xxxx - wyswietlenie xxxx ostatnich zadan" << endl;
    cout << " memo project yyyy - wyswietlenie zadan z listy" << endl;
    cout << " memo list - wyswietlanie zadan" << endl;
    cout << " memo list all - wyswietlanie zadan (z ukonczonymi)" << endl;
    cout << " memo list done - wyswietlanie zadan ukonczonych" << endl;
    cout << " memo list pending- wyswietlanie zadan w trakcie wykonywania" << endl;
    cout << " memo find xxxx - wyswietlenie zadania od ID" << endl;
    cout << " memo rm yy - usuwanie zadania o ID yy" << endl;
    cout << " memo new yy - ustawianie statusu zadania na nowe" << endl;
    cout << " memo pending yy - ustawianie statusu zadania na w trakcie wykonywania" << endl;
    cout << " memo done yy - ustawianie statusu zadania na wykonane" << endl;
    cout << " memo edit yy - tryb edycji zadania" << endl;
    cout << " memo i yy - zmiana priorytetu zadania o ID na normalny" << endl;
    cout << " memo ii yy - zmiana priorytetu zadania o ID na wysoki" << endl;
    cout << " memo iii yy - zmiana priorytetu zadania o ID na wyzszy" << endl;
    cout << " memo iiii yy - zmiana priorytetu zadania o ID na najwyzszy" << endl;
    cout << " memo five - ostatnie piec zadan" << endl;
    cout << " memo actual/todo - aktualne zadania o statusie w trakcie wykonywania" << endl;
    cout << endl;
    cout << "      @projekt" << endl;
}

int main(int argc, char** argv) {
    if (argc <= 1) {
        help();
        return 0;
    }

    char username[MAXLOGNAME];
    getlogin_r(username, MAXLOGNAME);

    bool appExit = true;
    do {
        // Loading database
        loadFromFile();

        //cout << termcolor::blue << termcolor::bold << "------------< Memo (" << tasks.size() << " tasks) >------------" << termcolor::reset << endl << endl;

        // Parsing arguments to list of words
        list<String> words;
        if (compareStrings(argv[1], "run")) {
            appExit = false;
            words.clear();

            char *wordsInput;
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

//        for (String &word : words) {
//            cout << "C: " << word.text << endl;
//        }

        //system("clear");
        //cout << termcolor::blue << termcolor::bold << "<>------------------< Memo >------------------<>" << termcolor::reset << endl << endl;

        Command command = T_NONE;
        int wordNumber = -1;

        taskNew.priority = NORMAL;
        taskNew.project = "";
        taskNew.taskName = "";
        taskNew.ID = tasks.front().ID + 1;
        taskNew.status = NONE;
        taskNew.tags = "";

        bool isReady = false;

        for (auto &word : words) {
            wordNumber++;
            if (wordNumber == 0) {
                //cout << " -- Command: " << it->text << endl;
                if (compareStrings(word.text, "add") || compareStrings(word.text, "a")) command = T_ADD;
                else if (compareStrings(word.text, "last") || compareStrings(word.text, "l")) command = T_LAST;
                else if (compareStrings(word.text, "list") || compareStrings(word.text, "ls")) command = T_LIST;
                else if (compareStrings(word.text, "find") || compareStrings(word.text, "s")) command = T_FIND;
                else if (compareStrings(word.text, "rm")) command = T_RM;
                else if (compareStrings(word.text, "new") || compareStrings(word.text, "n")) command = T_NEW;
                else if (compareStrings(word.text, "pending") || compareStrings(word.text, "p")) command = T_PENDING;
                else if (compareStrings(word.text, "done") || compareStrings(word.text, "d")) command = T_DONE;
                else if (compareStrings(word.text, "edit") || compareStrings(word.text, "e")) command = T_EDIT;
                else if (compareStrings(word.text, "i")) command = T_I;
                else if (compareStrings(word.text, "project") || compareStrings(word.text, "pr")) command = T_PROJECT;
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
                                        taskNew.tags = task.tags;
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
                    } else if (compareStrings(word.text, "--iiii")) {
                        taskNew.priority = HIGHEST;
                    } else if (compareStrings(word.text, "--iii")) {
                        taskNew.priority = HIGHER;
                    } else if (compareStrings(word.text, "--ii")) {
                        taskNew.priority = HIGH;
                    } else if (compareStrings(word.text, "--i")) {
                        taskNew.priority = HIGH;
                    } else if (compareStrings(word.text, "--normal")) {
                        taskNew.status = NONE;
                    } else if (compareStrings(word.text, "--pending")) {
                        taskNew.status = PENDING;
                    } else if (compareStrings(word.text, "--done")) {
                        taskNew.status = DONE;
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
                    if (compareStrings(word.text, "") || compareStrings(word.text, " ")) showTasks();
                    if (compareStrings(word.text, "pending") || compareStrings(word.text, "p")) showTasks(PENDING);
                    if (compareStrings(word.text, "done") || compareStrings(word.text, "d")) showTasks(DONE);
                    if (compareStrings(word.text, "all") || compareStrings(word.text, "a")) showTasks(ALL);
                    break;
                }

                if (command == T_I || command == T_II || command == T_III || command == T_IIII) {
                    if (command == T_I) editTaskPriority(taskNew.ID, NORMAL);
                    if (command == T_II) editTaskPriority(taskNew.ID, HIGH);
                    if (command == T_III) editTaskPriority(taskNew.ID, HIGHER);
                    if (command == T_IIII) editTaskPriority(taskNew.ID, HIGHEST);
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
                //tasks.push_front(taskNew);
                for (auto it = tasks.begin(); it != tasks.end(); it++) {
                    if (it->ID == taskNew.ID) {
                        it->taskName = taskNew.taskName;
                        it->status = taskNew.status;
                        it->project = taskNew.project;
                        it->priority = taskNew.priority;
                        it->date = taskNew.date;
                        it->tags = taskNew.tags;
                        break;
                    }
                }
                cout << termcolor::cyan << termcolor::bold << "#" << taskNew.ID << termcolor::reset << " edited" << endl;
                showTask(taskNew.ID);
            }
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

        // cout << endl;

        saveToFile();
        tasks.clear();
    } while (!appExit);

    return 0;
}