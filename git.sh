#!/bin/bash
date
echo "code add......"
git add .
echo "code commit......"
git commit -m "合入阻塞非阻塞代码"
echo "code push up......"
git push -u origin master

