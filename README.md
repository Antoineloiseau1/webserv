Sur les request POST (curl -X POST http://localhost:4242/caca), le serveur renvoi 201 Created, dans response.cpp ligne 386 on a un else if statement graaave chelou on sait pas ce que ca fait concretement, 
en tout cas pour faire le taf il suffierai de renvoyer error 422 (fichier html deja ecrit) dans le cas ou le body de POST est vide.
Kiss
