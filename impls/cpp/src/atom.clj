(def! cache (fn* [f]
    (let* [mem (atom {})]
        (fn* [& args]
            (if-let [e (find @mem args)]
                e
                (let* [ret (f ... args)] 
                    (do
                        (swap! mem assoc args ret)
                        ret)))))))

(def! fib (fn* [n]
    (cond
        [(<= n 1) n]
        [true (+ (fib (- n 1)) (fib (- n 2)))])))

(def! fib (cache fib))

(prn (list? *ARGV*))
(println (fib 6))