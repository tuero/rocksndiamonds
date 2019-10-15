/**
 * @file: base_option.h
 *
 * @brief: Base option which all implemented options should be derived from.
 * 
 * @author: Jake Tuero
 * Date: September 2019
 * Contact: tuero@ualberta.ca
 */

#ifndef BASE_OPTION_H
#define BASE_OPTION_H

#include <string>
#include <ostream>
#include <deque>

#include "option_types.h"

// Engine
#include "../../engine/action.h"
#include "../../engine/engine_types.h"
#include "../../engine/engine_helper.h"


/**
 * Skeleton definition which each derived Option is based off of.
 *
 * All options implemented should be derived from this class. An option is a policy
 * which executes (possible non-deterministic) actions. The controller should determine
 * the type of option to run depending on the state. The method run() are required to 
 * be implemented.
 */
class BaseOption {
protected:
    OptionType optionType_ = OptionType::Misc;
    enginetype::GridCell goalCell_;
    int spriteID_ = -1;
    std::deque<enginetype::GridCell> solutionPath;
    int timesCalled_ = 0;
    int counter_ = 0;

    struct Node {
        int id;
        int parentId;
        enginetype::GridCell cell;
        float g;
        float h;
    };

    class CompareNode {
        public:
            bool operator() (Node left, Node right) {
                return (left.g + left.h) > (right.g + right.h);
            }
    };

    void runAStar(bool flag);

    int girdCellIndex(const enginetype::GridCell cell);

public:

    /**
     * Run the action(s) defined by the option.
     * 
     * The definition of the options can be their own programs in the sense that they can
     * either step forward a single step, or perform many actions such as moving to a 
     * sprite of particular interest. It is up to the Option implementation to handle
     * events such as 
     */
    
    // This should auto increment timesCalled_
    virtual bool run() = 0;

    // On true, necessary internal variables should be reset.
    virtual bool singleStep(Action &action) = 0;

    virtual bool isValid_() = 0;

    bool isValid() {return isValid_() && enginehelper::isSpriteActive(spriteID_);}

    virtual int getTimesCalled() {return timesCalled_;}

    virtual OptionType getOptionType() {return optionType_;}

    virtual std::string optionToString() = 0;

    virtual std::ostream& toString(std::ostream& o) const {
        return o << "";
    }

    void setSpriteID(int spriteID) {spriteID_ = spriteID;}

    int getSpriteID() {return spriteID_;}

};



#endif  //BASE_OPTION_Hd


