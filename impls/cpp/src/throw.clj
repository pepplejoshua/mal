(def! faulty (fn* [] 
    (throw "(faulty) exception...")))

(try*
    (faulty)
(catch* ErrorStr
    (println (+ "Handled Exception =>> " 
                ErrorStr 
                " in catch* brace"))))

; uncaught exception
(faulty)