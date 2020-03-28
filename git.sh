#!/bin/bash
date
echo "code add......"
git add .
echo "code commit......"
git commit -m "合入wdt和seq_file_list驱动，其中seq_file_list有问题，需要定位"
echo "code push up......"
git push -u origin master

