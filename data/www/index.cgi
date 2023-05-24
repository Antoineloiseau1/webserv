#!/usr/bin/env python
import os

def generate_directory_listing(directory):
    # Récupérer la liste des fichiers dans le répertoire
    files = os.listdir(directory)

    # Générer le contenu HTML de la liste des fichiers
    html_content = "<h1>Listing des fichiers</h1>\n<ul>\n"
    for file in files:
        # Exclure les fichiers et répertoires spéciaux
        if file not in ['.', '..']:
            # Ajouter le nom du fichier à la liste HTML
            html_content += f"<li>{file}</li>\n"
    html_content += "</ul>"

    # Créer un fichier index.html et y écrire le contenu HTML
    with open('index.html', 'w') as file:
        file.write(html_content)

# Chemin du répertoire
directory = '/chemin/vers/le/repertoire'

# Générer la liste des fichiers et créer le fichier index.html
generate_directory_listing(directory)

