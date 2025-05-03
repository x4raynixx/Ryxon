RX is a lightweight and efficient scripting language designed for ease of use and low system impact. With a clean, beginner-friendly syntax, it allows developers to build applications quickly without overloading system resources. RX is ideal for lightweight scripting, automation, and development on machines with limited performance.
---

# 💪 Usage:
> Step 1: Create a .rx file
> 
> Step 2: Write some code
> 
> Step 3: Run it and have fun!

---

# 💻 Installation:
> Step 1: Go to releases
> 
> Step 2: Download the newest Installer from the newest Release
> 
> Step 3: Run the installer and wait to finish his job.
> 
> Step 4: Now RX Scripting Language is installed on your computer and you can use it!

---

# 🛠️ Build it yourself
Run the following command in your terminal:
```
git clone https://github.com/x4raynixx/RX-Scripting.git
cd RX-Scripting
```

Build for windows:
```
dotnet publish -c Release -r win-x64 -p:PublishSingleFile=true -p:IncludeAllContentForSelfExtract=true --self-contained true /p:PublishTrimmed=true /p:DebugType=None /p:DebugSymbols=false -o ./publish
```

Build for Linux:
```
dotnet publish -c Release -r linux-x64 -p:PublishSingleFile=true -p:IncludeAllContentForSelfExtract=true --self-contained true /p:PublishTrimmed=true /p:DebugType=None /p:DebugSymbols=false -o ./publish
```

Output: publish/rx.exe
