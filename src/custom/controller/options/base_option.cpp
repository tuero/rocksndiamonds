/**
 * @file: base_option.cpp
 *
 * @brief: Base option which all implemented options should be derived from.
 * 
 * @author: Jake Tuero
 * Date: September 2019
 * Contact: tuero@ualberta.ca
 */

#include "base_option.h"

#include <queue>
#include <map>

#include "../../engine/engine_helper.h"

std::ostream& operator<<(std::ostream& o, const BaseOption& baseOption) { 
    return baseOption.toString(o); 
}


int BaseOption::girdCellIndex(const enginetype::GridCell cell) {
    int width = enginehelper::getLevelWidth();
    return (cell.y * width + cell.x);
}  


void BaseOption::runAStar(bool flag) {
    enginetype::GridCell playerCell = enginehelper::getPlayerPosition();
    Node startNode = {girdCellIndex(playerCell), -1, playerCell, 0, (float)enginehelper::getL1Distance(playerCell, goalCell_)};
    // goalCell_ = enginehelper::getSpriteGridCell(spriteID_);

    std::priority_queue<Node, std::vector<Node>, CompareNode> openQueue;
    std::map<int, Node> openMap;
    std::map<int, Node> closed;

    openQueue.push(startNode);
    openMap[startNode.id] = startNode;

    while (!openQueue.empty()) {
        Node node = openQueue.top();
        openQueue.pop();
        openMap.erase(node.id);
        closed[node.id] = node;

        // Found goal cell
        if (node.cell == goalCell_) {
            solutionPath.clear();
            while(!(node.cell == playerCell)) {
                solutionPath.push_front(node.cell);
                node = closed[node.parentId];
            }

            if (flag) {
                for (enginetype::GridCell cell : solutionPath) {
                    enginetype::GridCell playerCell = enginehelper::getPlayerPosition();
                    Action action = enginehelper::getResultingAction(playerCell, cell);

                    enginehelper::setEnginePlayerAction(action);
                    enginehelper::engineSimulate();
                }

                solutionPath.clear();
            }

            break;
        }

        // Expand children
        for (Action action : ALL_ACTIONS) {
            if (action == Action::noop) {continue;}

            enginetype::GridCell childCell = node.cell;
            if (action == Action::left) {childCell.x -= 1;}
            else if (action == Action::right) {childCell.x += 1;}
            else if (action == Action::up) {childCell.y -= 1;}
            else if (action == Action::down) {childCell.y += 1;}

            if (!enginehelper::isWalkable(action, node.cell) && !(childCell == goalCell_)) {continue;}

            Node childNode = {girdCellIndex(childCell), -1, childCell, 0, (float)enginehelper::getL1Distance(childCell, goalCell_)};
            float newG = node.g + std::abs(node.h - childNode.h) + 1;

            if (openMap.find(childNode.id) != openMap.end()) {
                if (childNode.g <= newG) {continue;}
                childNode.g = newG;
                childNode.parentId = node.id;
            }
            else if (closed.find(childNode.id) != closed.end()) {
                if (childNode.g <= newG) {continue;}
                childNode.g = newG;
                childNode.parentId = node.id;
                closed.erase(childNode.id);

                openQueue.push(childNode);
                openMap[childNode.id] = childNode;
            }
            else {
                childNode.g = newG;
                childNode.parentId = node.id;
                openQueue.push(childNode);
                openMap[childNode.id] = childNode;
            }
        }
    }
}