///
/// \file path_tree.hpp
///
///  Created on: Jun 17, 2014
///      Author: rdumitriu at gmail.com

#ifndef GERYON_PATH_TREE_HPP_
#define GERYON_PATH_TREE_HPP_

#include <string>
#include <set>
#include <vector>

#include "string_utils.hpp"

namespace geryon { namespace server {

namespace detail {

    template <typename T>
    struct PathTreeNode {
        std::string pathFragment;
        std::set<T> data;
        std::vector<PathTreeNode<T>> children;

        PathTreeNode() {}
        ~PathTreeNode() {}

        PathTreeNode<T> * getChild(const std::string & fragment) {
            for(unsigned int i = 0; i < children.size(); ++i) {
                PathTreeNode<T> * pc = &(children.at(i));
                if(pc->pathFragment == fragment) {
                    return pc;
                }
            }
            return 0;
        }

        PathTreeNode<T> * addChild(const std::string & fragment) {
            PathTreeNode<T> child;
            child.pathFragment = fragment;
            children.push_back(std::move(child));
            return &(children.at(children.size() - 1));
        }

        inline void addData(const T & d) {
            data.insert(d);
        }
    };

}

template <typename T>
class PathTree {
public:
    PathTree() {}
    ~PathTree() {}

    PathTree(const PathTree & other) = delete;
    PathTree & operator = (const PathTree & other) = delete;

    void addNode(const std::string & path, const T & data) {
        std::vector<std::string> splitPath = geryon::util::split(path, "/");
        detail::PathTreeNode<T> * pNode = &root;
        for(const std::string & pathFragment : splitPath) {
            if(pathFragment.find('*') != std::string::npos) {
                break;
            }
            detail::PathTreeNode<T> * pChild = pNode->getChild(pathFragment);
            if(!pChild) {
                pChild = pNode->addChild(pathFragment);
            }
            pNode = pChild;
        }
        pNode->addData(data);
    }

    std::vector<T> getDataForPath(const std::string & path) {
        std::vector<std::string> splitPath = geryon::util::split(path, "/");
        std::vector<T> ret;
        detail::PathTreeNode<T> * pNode = &root;
        for(const std::string & pathFragment : splitPath) {
            detail::PathTreeNode<T> * pChild = pNode->getChild(pathFragment);
            if(!pChild) {
                return ret;
            }
            if(!pChild->data.empty()) {
                ret.insert(ret.end(), pChild->data.begin(), pChild->data.end());
            }
            pNode = pChild;
        }
        return ret;
    }

private:
    detail::PathTreeNode<T> root;
};

} }

#endif
