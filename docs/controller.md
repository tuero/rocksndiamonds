
# Creating a Basic Controller
To add your own controller, create a folder to hold your controller files under `./src/ai/controller/YOUR_CONTROLLER_NAME_HERE`. Inside the folder, create your controller class and any subsequent source files required. All controllers must inherit the `base_controller` class. To see the details for the `base_controller`,  refer to the header file located under `./src/ai/includes`. Other than a constructor, the only method required to be implemented is the `getAction()` method, which tells the engine handler what action to perform next. Although not required, it is highly recommended to overload the `controllerDetailsToString()` method as well, as it is useful for debugging clarity.

An example of a default controller, which returns the do-nothing action, is implemented below. The source file for this example can also be seen under `./src/ai/controller/default/`.

```cpp
// Includes
#include "base_controller.h"
#include "engine_types.h"


/**
 * Default controller
 *
 * This does nothing, and acts as a backup to prevent unknown controller input from breaking
 */
class Default : public BaseController {
private:

public:

    Default();

    /**
     * Always returns the noop action.
     */
    Action getAction() override {return Action::noop;};

    /**
     * Convey any important details about the controller in string format.
     * Useful for logging relevant information about the current controller configuration.
     */
    virtual std::string controllerDetailsToString() override {return "My controller";}
};
```

## Connecting the controller to the engine
To let the engine know the existence of your controller, a few changes need to be made.
1. Edit the `./src/ai/include/controller_listing.h` to include information about your controller. A new entry should be made in the `enum ControllerType`, as well as a short name in `CONTROLLER_STRINGS`. The ordering should match **BOTH** in the enum as well as the string list. The name given in the string list will be used as the command line argument when selecting the controller, i.e.
```shell
./rocksndiamonds_gui -controller YOUR_CONTROLLER_NAME
```
2. The last step is to set the global controller pointer to your new class. Open `./src/ai/controller/controller.cpp`, and include your controller header at the top. Next, add another conditional statement inside `initController(ControllerType controller)` which will call your controller's constructor, and pass in any arguments required. **NOTE**: The conditional checks against `enum ControllerType`, so make sure you use the same enum name that was added in the above.
```cpp
/**
 * Inits the controller.
 * Controller type is determined by command line argument.
 */
void Controller::initController(ControllerType controller) {
    // Set appropriate controller
    if (controller == CONTROLLER_DEFAULT) {
        baseController_ = std::make_unique<Default>();
    }
    ...
    else if (controller == YOUR_CONTROLLER_ENUM_NAME) {
        baseController_ = std::make_unique<YourControllerClass>();
    }
    ...
}
```

## Calling your controller
Once the above steps are done, you should now be able to call your controller. Controllers are set using the `-controller YOUR_CONTROLLER_STRING_NAME` command line argument. For example, if you added an entry to `CONTROLLER_STRINGS` called `MY_CONTROLLER`, the program call would be:
```shell
./rocksndiamonds_gui -controller MY_CONTROLLER
```