
#!/usr/bin/env python3

import sys

# Read raw POST input
input_data = sys.stdin.read()

# Parse plain-text input like "name=John\nage=30"
data = dict(item.split('=') for item in input_data.splitlines() if '=' in item)

name = data.get("name", "Unknown")
age = data.get("age", "Unknown")


# Generate HTML
html_content = f"""
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>User Info</title>
    <link href="https://fonts.googleapis.com/css2?family=Inter:wght@400;600;700&display=swap" rel="stylesheet"/>
    <link rel="stylesheet" href="/style.css"/>
    <style>
        body {{
            background-color: var(--background-color);
            color: var(--text-color);
            font-family: 'Inter', -apple-system, BlinkMacSystemFont, sans-serif;
        }}
        .wrapperAll {{
            margin-top: 5rem;
            display: flex;
            justify-content: center;
        }}
        .wrapper {{
            background: var(--container-bg);
            padding: 2rem;
            border-radius: 12px;
            box-shadow: 0 8px 32px var(--shadow-color);
            text-align: center;
            transition: transform 0.3s ease;
            border: 1px solid var(--border-color);
        }}
        .wrapper h2 {{
            color: var(--primary-color);
            font-size: 2rem;
            margin-bottom: 1rem;
        }}
        .line {{
            height: 3px;
            background: linear-gradient(90deg, var(--primary-color), var(--secondary-color));
            width: 50px;
            margin: 1rem auto;
        }}
        p {{
            font-size: 1.2rem;
            color: var(--text-color);
            opacity: 0.8;
        }}
        .button {{
            display: inline-block;
            background: var(--button-bg);
            color: var(--button-text);
            padding: 0.75rem 1.5rem;
            border-radius: 12px;
            font-weight: 600;
            text-decoration: none;
            margin-top: 2rem;
            text-align: center;
            cursor: pointer;
            transition: background 0.3s ease, transform 0.2s ease;
        }}
        .button:hover {{
            background: var(--button-hover);
            transform: translateY(-0.125rem);
        }}
    </style>
</head>
<body>
    <section class="wrapperAll container">
        <div class="wrapper">
            <h2>User Details</h2>
            <div class="line"></div>
            <p>Hello, <strong>{name}</strong>!</p>
            <p>You are <strong>{age}</strong> years old.</p>
            <a href="/" class="button">Back to Home</a>
        </div>
    </section>
</body>
</html>
"""

print(html_content)
