<h3>Environment</h3>
<p>Qt 5.9.0<br>MinGW 5.3.0 32bit for C++</p>

<h3>SmallC language:</h3>

    1) <program>∷={<declaration_list><statement_list>} 
    2) <declaration_list>∷= <declaration_list><declaration_stat>|<declaration_list>|ε 
    3) <declaration_stat>∷=int ID;
    4) <statement_list>∷=<statement_list>|<statement>|<statement>|ε
    5) <statement>∷=<if_stat>|<while_stat>|<read_stat>|<write_stat>|<compound_stat>|<expression_stat>
    6) <if_stat>∷=if (<expression>) <statement> [else <statement>]
    7) <while_stat>∷=while (<expression>) <statement>
    8) <write_stat>∷=write <expression>;
    9) <read_stat>∷=read ID;
    10) <compound_stat>∷={<statement_list>} 
    11) <expression_stat>∷=<expression>;|;
    12) <expression>∷=ID=<bool_expr>|<bool_expr>
    13) <bool_expr>∷=<additive_expr>|<additive_expr>(>|<|>=|<=|==|!=)<additive_expr>    
    14) <additive_expr>∷=<term>{(+|-)<term>}
    15) <term>∷=<factor>{(*|/)<factor>}
    16) <factor>∷=(<expression>)|ID|NUM
