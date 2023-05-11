#!/usr/bin/env python

import cgi


# Set the Content-type header to indicate that this is an HTML document
print("HTTP/1.1 200 OK\r\nContent-type: text/html\nContent-Length: 10003\r\nConnection: close\r\n\r\n")

# Create an instance of the FieldStorage class to parse the form data
form = cgi.FieldStorage()

# Extract the values of the form fields
nom = form.getvalue('nom')
prenom = form.getvalue('prenom')
email = form.getvalue('email')
ville = form.getvalue('ville')

# Output an HTML document that includes the submitted data
print("<html>")
print("<head>")
print("<title>Form Submission Results</title>")
print("</head>")
print("<body>")
print("<h1>Form Submission Results</h1>")
print("<p>Thank you for submitting the form. Here is the information you provided:</p>")
print("<ul>")
print("<li><strong>Nom:</strong> " + nom + "</li>")
print("<li><strong>Prenom:</strong> " + prenom + "</li>")
print("<li><strong>Email:</strong> " + email + "</li>")
print("<li><strong>Ville:</strong> " + ville + "</li>")
print("</ul>")
print("</body>")
print("</html>")
