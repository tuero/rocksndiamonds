
#ifndef CUSTOM_H
#define CUSTOM_H



#ifdef __cplusplus
extern "C"
{
#endif
    // C declarations
    #include "../main.h"
    #include "../screens.h"
    void findPath(void);
    void printBoardState(void);
    void setLevel(int levelset); 
    int getAction(void);

#ifdef __cplusplus
}
#endif






#endif  //CUSTOM_H


