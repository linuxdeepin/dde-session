#ifndef KEYRING_CHECKER_H
#define KEYRING_CHECKER_H

/**
 * @brief The KeyringChecker class
 * @brief 当无法从 PAM 拿到用户密码时（免密码登录、自动登录）， pam_gnome_keyring.so 不会自动创建「登录」钥匙环。
 * 若无默认钥匙环，则自动创建一个无密码的钥匙环。
 */
class KeyringChecker
{
public:
    void init();
};

#endif // KEYRING_CHECKER_H
