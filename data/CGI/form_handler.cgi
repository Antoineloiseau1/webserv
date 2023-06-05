#!/usr/bin/env python

import cgi
import sys

# Create an instance of the FieldStorage class to parse the form data
form = cgi.FieldStorage()

# Extract the values of the form fields
nom = form.getvalue('nom', '')
prenom = form.getvalue('prenom', '')
email = form.getvalue('email', '')
ville = form.getvalue('ville', '')

# Set the Content-type header to indicate that this is an HTML document

# Output an HTML document that includes the submitted data
print("<html>")
print("<head>")
print("<title>Form Submission Success</title>")
print("</head>")
print("<body>")
print("<h1>Form Submission Success</h1>")
print("<p>Thank you for submitting the form. Here is the information you provided:</p>")
print("<ul>")
print("<li><strong>Nom:</strong> " + nom[6:] + "</li>")
print("<li><strong>Prenom:</strong> " + prenom + "</li>")
print("<li><strong>Email:</strong> " + email + "</li>")
print("<li><strong>Ville:</strong> " + ville + "</li>")
print("</ul>")
print("</body>")
print("</html>")

