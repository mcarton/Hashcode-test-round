import parse

def make_part(p, t):
    ((l1, c1), (l2, c2), (l3, c3)) = t
    col1 = min(c1,c2,c3)
    col2 = max(c1,c2,c3)
    lig1 = min(l1,l2,l3)
    lig2 = max(l1,l2,l3)

    return (lig1, col1, lig2, col2);


def close_ham(p, y, x):
    k = 3
    infi = max(0, y-k)
    infj = max(0, x-k)
    supi = min(p.rows, y+k)
    supj = max(p.cols, x+k)

    hams = []
    for i in range(infi, supi):
        for j in range(infj, supj):
            if(p.problem[i][j] == 'H'):
                hams.append((i,j))

    hams = sorted(hams, key=lambda h: norme1((i, j), h))

    if len(hams) > 2:
        return hams[0:2]
    else:
        return hams


