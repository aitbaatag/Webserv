#!/usr/bin/env python3

import cgi
import html

# Parse form data
form = cgi.FieldStorage()
username = form.getvalue("username", "Guest")  # Default to "Guest" if not provided

# Escape HTML characters to prevent injection
username = html.escape(username)

# **HTML Output**
print(f"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Welcome</title>
    <style>
        body {{
            font-family: 'Segoe UI', sans-serif;
            background-color: #f0f8ff;
            color: #333;
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100vh;
            text-align: center;
        }}
        .container {{
            max-width: 400px;
            padding: 40px;
            background-color: white;
            border-radius: 10px;
            box-shadow: 0 10px 30px rgba(0, 123, 255, 0.1);
        }}
        .title {{
            font-size: 36px;
            font-weight: bold;
            color: #007bff;
        }}
        .subtitle {{
            font-size: 18px;
            color: #555;
            margin-bottom: 20px;
        }}
        .form-group {{
            margin-top: 20px;
        }}
        button {{
            padding: 12px 24px;
            background-color: #007bff;
            color: white;
            font-weight: bold;
            border: none;
            border-radius: 5px;
            transition: background-color 0.3s ease;
        }}
        button:hover {{
            background-color: #0056b3;
        }}
    </style>
</head>
<body>
    <div class='container'>
        <div class='title'>Welcome, {username}!</div>
        <p class='subtitle'>You have successfully logged in.</p>
        <p>If you want to upload a file, click the button below:</p>
        <div class='form-group'>
            <form action='/upload.html' method='GET'>  <!-- ✅ Redirects to upload.html -->
                <button type='submit'>Upload</button>
            </form>
        </div>
        <a href='/login.html' style='display: block; margin-top: 15px; text-decoration: underline; color: #007bff;'>Go Back</a>
    </div>
</body>
</html>
""")
