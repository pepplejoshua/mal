(def! test (fn* [n & key]
    (match n
        [(:List a & b) (list a ... b)]
        [(:Vector a true c) (list n a c)]
        [(:Pair a b) (+ a b)]
        [:HashMap (find n ... key)]
        [:Symbol (eval n)]
        [:Keyword n]
        [:String (+ n " is a String")]
        [:Nil n]
        [:Boolean (not n)]
        [:Int (* n 2)]
        [:Func n] ; this handles both Builtins and User created functions
        [:Atom @n]
        [:All (type n)]))) ; a catchall
(def! sym 200)
(println (str "1.") (test '(1 2 3 4 5))) ; prints (1 2 3 4 5)
(println (str "2.") (test [200 true nil])) ; prints (200 true nil) 200 nil
(println (str "3.") (test (cons "1" "2"))) ; prints "Joshua Pepple"
(println (str "4.") (test {"s" true "o" false} "s")) ; prints true
(println (str "5.") (test 'sym)) ; prints 200
(println (str "6.") (test :kw)) ;prints kw
(println (str "7.") (test "This")) ; prints "This is a String"
(println (str "8.") (test nil)) ; prints nil
(println (str "9.") (test false)) ; prints true
(println (str "10.") (test 22)) ; prints 44
(println (str "11.") (test test)) ; prints {TCOptFunction test}
(println (str "12.") (test (atom "some atom"))) ; prints "some atom"