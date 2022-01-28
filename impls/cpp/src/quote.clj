(def! a 3)
(def! b 2)
(def! c 1)

(def! call '(+ a b c))

(def! count (atom 0))

(prn (swap! count + 1))
(prn (swap! count + 1))
(prn (swap! count + 1))