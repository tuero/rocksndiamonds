#!/bin/bash
# SOURCE_DATE_EPOCH=$(test -d ../.git && test `git ls-files -m | wc -l` -eq 0 && git show -s --format=%ct || date +%s)
# str_date=$(date -u -d "${SOURCE_DATE_EPOCH}" "+%Y-%m-%d %H:%M"  2>/dev/null || date -u -r "${SOURCE_DATE_EPOCH}" "+%Y-%m-%d %H:%M" 2>/dev/null || date -u "+%Y-%m-%d %H:%M")
# echo "#define SOURCE_DATE_STRING \"${str_date}\"" > conftime.h

TIMESTAMP_FORMAT="%Y-%m-%d %H:%M"
SOURCE_DATE_EPOCH=$(test -d ../.git && test `git ls-files -m | wc -l` -eq 0 && git show -s --format=%ct || date +%s)
str_date=$(date -u -d "@${SOURCE_DATE_EPOCH}" "+${TIMESTAMP_FORMAT}" 2>/dev/null || date -u -r "${SOURCE_DATE_EPOCH}" "+${TIMESTAMP_FORMAT}" 2>/dev/null || date -u "+${TIMESTAMP_FORMAT}")
echo "#define SOURCE_DATE_STRING \"${str_date}\"" > conftime.h
