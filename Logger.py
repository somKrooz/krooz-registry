import logging

def Log(data: str) -> None:
  logging.log(data)

def Info(data: str) -> None:
  logging.info(data)

def Debug(data: str) -> None:
  logging.debug(data)
