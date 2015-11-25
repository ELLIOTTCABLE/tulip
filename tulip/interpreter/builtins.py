import tulip.interpreter.lang as core

dispatch = {}

# *shrugs* i wanted to fold a predicate but
def allNumbers(list):
    for i in list:
        if isinstance(i, core.Literal) and isinstance(i.value, (int, long, float)):
            continue
        else:
            return False
    return True

def literal(list):
    for i in list:
        if isinstance(i, core.Literal):
            continue
        else:
            return False
    return True

dispatch["+"] = {"arity": 2, "definition": (lambda xs: xs[0].value + xs[1].value), "check": allNumbers}
dispatch["-"] = {"arity": 2, "definition": (lambda xs: xs[0].value - xs[1].value), "check": allNumbers}
dispatch["print"] = {"arity": 1, "definition": (lambda xs: p(xs)), "check": literal}

def p(xs):
    for x in xs:
        print x.value
    return core.Tag("ok")