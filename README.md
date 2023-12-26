# Introduction
This introduces an app whose purpose is to help people learn a catechism and/or a ceremonial text.

The basic idea is that the user would use the app to step through the text, learning each line as it comes and perhaps trying to speak the next line out loud before advancing.

# Usage

On loading the app, the user is greeted with a screen, shown below, asking them to drag and drop a text file containing the document they wish to learn.  More details about the required formatting of this document can be found below.

![Welcome screen](./Images/welcome_screen.png "Welcome screen")

Dragging and dropping such a file onto the window leads to the screen shown in figure 2. Note that this screen is shown for a catechism imagined by ChatGPT for a purely fictional religion.

![After loading a document](./Images/initial_screen_after_loading.png "After first loading")

The user then steps through the text using the "Next" and "Back" buttons, and may restart at the beginning of the text with the "Reset" button.

![Working with a text](./Images/scrollable_screen.png "Stepping through the text")

# Protection of secrets

As stated above, the catechism files included in this repository were imagined by ChatGPT for a purely fictional relgion.  It will *never* be the case that genuine, non-imagined files will be stored here, nor will such information ever be included in the source code.  This repository is only for the storage and distribution of the source code and associated executables, and these executables are useless if the user is unable to provide suitable texts.

It is often the case that texts that users wish to learn are private or secret, perhaps due to vows made to a lodge.  For this reason this application allows for the decryption of specially encrypted documents.

The encryption used in this application is AES-256, and is considered state of the art.

![Password-based decryption of a text](./Images/password_request.png "Providing a password")

Entering the wrong password will return the user to the password screen.

The correct password opens the document as for an unecrypted text (see figure 2).

# Format of the input file

The input file (either unencrypted or after decryption) should be encoded as UTF-8 and is expected to follow a very simple format.

The first line provides the title of the document, such as the one shown in figure 2. The remainder of the file is then divided line-by-line to be served up to the user one at a time as they click through the text.

