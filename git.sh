#!/bin/bash
date
echo "code add......"
git add .
echo "code commit......"
git commit -m "adc代码，非阻塞read未测试"
echo "code push up......"
git push -u origin master

