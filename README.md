# Server-side app for readsProfiler.

## [Functionality of the App]

The app allows the client to:
  - Log in
  - Create new accounts
  - Search for books (with autocomplete/autocorrection in case of spelling errors)
  - Get recommandations for books, based on their previous interests and downloaded books.
  - Download the books.

> ***To see all of this in action, head over to the readProfiler's [Client-Side App page](https://github.com/mateipruteanu/readsProfiler_Client).***



## [Technical Details]

### Justification for the technologies used

The server app is built using C++ and is made **concurrent using threads** for each new client that connects to it. It stores data about the users (such as username, password, preferences, read books), and book information (such as title, author, genre, and ISBN) in XML files. The books in the XML file correspond to the .mobi files of the books from a separate folder.

**TCP** was used to ensure that the data being sent is not altered through transporting it from the client to the server and back. Speed is not a concern, since the heaviest workload would be downloading a book, which is at most a few MBs, and users realistically will only download a book every few seconds at most. UDP would have been impractical and almost impossible to use, since it would prove unreliable for this type of application (because the app would require a constant and reliable connection to the server), and those negatives outweigh the positives (such as speed) for this use case.
The server is made concurrent using threads because this way, the data would be shared between the threads (such as user and book data), so that if it’s updated by one client (so one thread), all the others have instant access to the latest modifications.

### The Application Diagram

![Picture 1](https://user-images.githubusercontent.com/35728927/213144867-23f47a62-79d8-452c-aaea-403b2516081b.jpg)


### Treatment of Edge Cases

Some edge cases would be when the user presses search without writing anything in any of the input fields (or only writes an author and not a title/ISBN). In those cases, the client app detects that and displays a message telling the user to input the corresponding information. No command is sent to the server until enough information about the book is added by the user (such as a title or ISBN).
Another edge case would be when the client tries to create a new account using a username which is already in use. The server checks through all the usernames, and if it finds a matching, it sends back the message “account_already_exists”.
Also, if the passwords don’t match when creating a new account, no command is sent to the server, and the client is informed about the problem.
Another edge case would be if two different clients tried writing to the “tempUsers.xml” file at the same time. To prevent this, we use mutex locks to lock the file before opening it for writing and unlocking it after closing it.






