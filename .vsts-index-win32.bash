#!/bin/bash
echo urde-$1-win32-$2-sse2.exe > $3
echo urde-$1-win32-$2-sse41.exe >> $3
echo urde-$1-win32-$2-avx.exe >> $3
echo urde-$1-win32-$2-avx2.exe >> $3
