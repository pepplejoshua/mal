(defmacro! setq (fn* [v1 v2 e] 
    (list 'do 
            (list 'def! v1 e) 
            (list 'def! v2 e))))

; this line 
(def! [a b] [-1 -1])
; has similar semantic implications as
(prn (macroexpand (setq 'a 'b -1)))
; they both bind 2 Symbols (a, b) to -1