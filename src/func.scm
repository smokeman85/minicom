(define (toh val)
  (string-append "0x" (number->string val 16)))

(define (tob val)
  (string-append "0b" (number->string val 2)))
