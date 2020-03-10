# Memo

Works on macOS & Linux (not tested on other platforms). Only polish language available (for this moment) but it's really simple. 

![Memo Screenshot](https://github.com/Pelski/Memo/blob/master/screenshot.png?raw=true)

**Features:**

 - priority: 0/1/2/3
 - projects
 - tags
 - status: new/pending/done
 - sorting
 - filters
 - help
 
This is my old project. I use it every day and it works very well. Now I want to share it! Check it out.

**Installation:**

Change username (`#define USERNAME "pelski"`) and database path (`#define TASKS_FILE "/Users/pelski/.tasks.db"`) in `main.cpp` file.

```
sudo apt-get install g++
sudo apt-get install libboost-all-dev
sudo apt-get install sqlite3
g++ main.cpp -o memo  -L/usr/local/lib/ -lboost_filesystem -lsqlite3
chmod +x memo
./memo
```

---

Used: 
- TermColors - https://github.com/ikalnytskyi/termcolor/blob/master/include/termcolor/termcolor.hpp
- SQLite - https://www.sqlite.org/index.html
