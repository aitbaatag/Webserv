#!/usr/bin/env python3

import sys
import json

input_data = sys.stdin.read()

pairs = []
for sep in ['&', '\n']:
    if sep in input_data:
        pairs = input_data.split(sep)
        break
else:
    pairs = [input_data]

data = {}
for item in pairs:
    if '=' in item:
        key, value = item.split('=', 1)
        data[key.strip()] = value.strip()

name = data.get("name", "Unknown")
age = data.get("age", "Unknown")

# print("Content-Type: application/json\n")
print(json.dumps({"name": name, "age": age}))