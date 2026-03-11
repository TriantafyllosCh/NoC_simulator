Network on Chip Simulation

Οι παράμετροι διαμόρφωσης της συμπεριφοράς του Network on Chip (NoC) βρίσκονται αποκλειστικά στο αρχείο main_NoC_Sim.cc. Συγκεκριμένα, ορίζονται εντός του αντικειμένου simulation της κλάσης NoC (γραμμές 12-16) (δηλαδή οι αντίστοιχες ρυθμίσεις σε κάποιο interface).

Ο compiler που χρησιμοποίησα είναι ο gcc.
Όσο αφορά το Compile του κώδικα η εντολή είναι : g++ main_NoC_Sim.cc functions.cc classes.cc -o sim.exe
Για το execution 			       : ./sim.exe