- Reference and value arguments [X]
- programming documentation properly for each public methods, constructor,... [X]
- class diagram with encapsulation, interfaces, extendability (different colors), polymorphism (TreeBuilder) [X]
- Warning logs for styling when not closed [X]
- sequential diagram [X]

Dobrý den,
doladil jsem detaily na základě naší konverzace pár týdnů zpět.
Jednak program již vypisuje warningy, pokud zjistí, že stylizovaný text (hvězdičky) nebo kus kódu (zpětné apostrofy) nebyly uzavřeny před koncem řádku. Jako důkaz posílám fotku, můžete si to ale i ověřit, pokud kód spustíte na testovém příkladu `complex_table.md` (spuštění přesně tohoto příkladu jsem popsal navíc v README.md i s warningy, které by měly být vidět).
Dále jsem upravil diagram tříd, aby zobrazoval potencionální rozšíření přes nové interfacy. Navíc jsem přidal i jednoduchý sekvenční diagram zobrazující běh programu.
Nakonec jsem dokončil programátorskou dokumentaci v kódu a každá třída, konstruktor a veřejná metoda (plus důležité privátní metody) jsou zdokumentované.