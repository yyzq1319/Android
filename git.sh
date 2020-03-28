#!/bin/bash
date
echo "code add......"
git add .
echo "code commit......"
git commit -m "合入wdt_irq代码"
echo "code push up......"
git push -u origin master

