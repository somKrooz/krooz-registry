import typer
from .Manager import search_query, get_packages_by_ext , create_download ,fuzzy_find ,CreateUrl,LookForRelations,HardDownload
from .admin import Push , Delete

app = typer.Typer()

# Todo:
# 1. Make Exts Visible In fzf type of stuff
# 2. Make --force work with all Exts
# 5. Make It More Modular


@app.command()
def search(ext: str = typer.Option(..., "--ext", "--e", "-e", help="Download Packages Loacally")):
    if ext:
      get_packages_by_ext(ext)
    else:
      Arr = search_query()
      for ext , package in Arr.items():
        typer.echo(f"* {package} -> {ext} ")

@app.command()
def get(name: str = typer.Option(..., "--n", "--name", "-n", help="Package Name"),
        force: bool = typer.Option(False, "--force", "-f", help="Try to fetch related .h/.cpp files even if missing")
        ):
    if not force:
      create_download(str(name))
    else:
       Relations = LookForRelations(str(name))
       print(Relations)
       for i in Relations:
          HardDownload(i)

@app.command()
def fzf(name: str = typer.Option(..., "--name", "--n", "-n", help="Fuzzy Find Packages")):
  Packages = search_query()
  fuzzy_find(Packages , name)

@app.command()
def geturl(name: str = typer.Option(..., "--name", "--n", "-n", help="Get Git Url")):
  Name = CreateUrl(name)
  typer.echo(Name)

@app.command()
def push(name:  str = typer.Option(..., "--name", "--n", "-n"),
           commit: str = typer.Option(..., "--commit", "--c", "-c")):

  Push(name , commit)

@app.command()
def remove(name:  str = typer.Option(..., "--name", "--n", "-n"),
           commit: str = typer.Option(..., "--commit", "--c", "-c")):

  Delete(name , commit)

def main():
   app()

if __name__ == "__main__":
    main()
