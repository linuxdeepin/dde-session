#!/bin/bash

# 更新 dde-login-reminder 翻译文件
lupdate tools/dde-login-reminder -ts -no-obsolete translations/dde-login-reminder.ts

# tx push -s -f --branch m20
