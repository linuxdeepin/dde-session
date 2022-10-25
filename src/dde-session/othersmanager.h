#ifndef OTHERS_MANAGER_H
#define OTHERS_MANAGER_H

/**
 * @brief The OthersManager class
 * @brief 处理一些比较琐碎不好归类的事情，后面功能做的差不多了再梳理
 */
class OthersManager
{
public:
    OthersManager();

    void init();

private:
    void launchWmChooser();
    void launchKwin();
    void launchSessionDaemon();

    bool inVM();
};

#endif // OTHERS_MANAGER_H
