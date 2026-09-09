# Luminex Winlator 0.1

Launcher Windows leggero progettato per l'esecuzione dentro Winlator 11.

## Funzioni

- creazione di profili di navigazione persistenti e separati;
- apertura simultanea di profili diversi per usare account Gmail differenti;
- sessione incognito con cartella temporanea distinta;
- cancellazione automatica dei dati temporanei quando il processo del browser termina;
- supporto a finestre, mouse e tastiera fornito dal browser e da Winlator;
- nessuna password letta o salvata da Luminex.

## Browser richiesto

Luminex cerca `chrome.exe` o `chromium.exe` nelle posizioni comuni del container. In alternativa, copia un browser portatile nella cartella `browser` accanto a `Luminex-Winlator.exe`.

Il motore Chromium non viene incluso nell'artifact 0.1. Il browser deve essere ottenuto da una fonte ufficiale o affidabile e deve essere compatibile con Wine/Winlator.

## Uso

1. Estrai completamente `Luminex-Winlator-0.1.zip` nell'unità del container.
2. Installa Chrome/Chromium nel container oppure inseriscilo nella cartella `browser`.
3. Avvia `Luminex-Winlator.exe`.
4. Crea un profilo per ogni identità, ad esempio `Personale`, `Associazione` e `Scuola`.
5. Accedi al relativo account Gmail dentro ciascun profilo.
6. Usa `Nuova finestra incognito` quando non vuoi conservare la sessione.

## Limiti della versione 0.1

- Luminex gestisce e avvia il motore del browser, ma non incorpora ancora Chromium.
- La compatibilità effettiva dipende dalla versione del browser e dalla configurazione del container Winlator.
- La cancellazione della sessione incognito avviene dopo la chiusura del processo associato. Se Winlator viene terminato forzatamente, una cartella temporanea può restare e deve essere verificata nella versione successiva.
- La compatibilità con Gmail deve essere verificata con un account di prova prima di usare account importanti.
