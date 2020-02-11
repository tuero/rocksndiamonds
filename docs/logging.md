
# Logging

The portable header-only logging library [Plog](https://github.com/SergiusTheBest/plog) is used throughout this project. For detailed information, see the Plog page [here](https://github.com/SergiusTheBest/plog).

## Setting the log level
The log severity levels follow the same definition of Plog
```cpp
enum Severity
{
    none = 0,
    fatal = 1,
    error = 2,
    warning = 3,
    info = 4,
    debug = 5,
    verbose = 6
};
```
By default, the logging level is set to `error`. To specify the log level, use the `-loglevel` argument, along with the `Severity` code number as defined above. For example, to set the logging level to `info`,
```
./rocksndiamonds -controller MCTS -levelset hard_deadlock -loadlevel 3 -loglevel 4
```

## Using the logger
To use the logger, the header [logger.h](../src/ai/include/logger.h) needs to be included in your source file (which is located in the `include` folder). Two logging appenders are defined:
- `logger::FileLogger`: This logs to an output file located in `./src/ai/logs`. The naming scheme follows the same formatting as replay file, that is, the datetime and the PID of the client.
- `logger::ConsoleLogger`: This logs output to the console.

The formatting of the file/console logger is the same, but the file logger will contain more verbose information than the console. The basic usage of the logger is `PLOGLEVEL_(appender) << "..."`, where the `LEVEL` is the first letter of the `Severity`, the appender is as defined above, and the output message follows the stream output operator.

Example: logging a message to the console at the error severity:
```cpp
#include "logger.h"
...
PLOGE_(logger::ConsoleLogger) << "My message here.";
```

Example: logging a message to the file at the info severity:
```cpp
#include "logger.h"
...
PLOGI_(logger::FileLogger) << "My message here.";
```