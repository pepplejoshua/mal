(def! ackermann (fn* [m n]
    (cond
        [(= m 0) (+ n 1)]
        [(= n 0) (ackermann (- m 1) 1)]
        [true (ackermann (- m 1)
                         (ackermann m (- n 1)))])))

(def! countnodes (fn* [tree]
    (cond
        [(empty? tree) 0]
        [true (let* [top (first tree) rem (rest tree)]
                (cond
                    [(seq? top) (+ (countnodes top) (countnodes rem))]
                    [true (+ 1 (countnodes rem))]))])))
                

(def! tree  '(a (b (d) (e)) (c (f) (g))))
(prn (countnodes tree)) ; should print 7