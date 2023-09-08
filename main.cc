#include "parser.h"
#include "grammar.h"
#include "checker.h"

int main(int argc, char **argv) {

    std::string peg = R"(
# This code is hereby placed in the public domain.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHORS ''AS IS'' AND ANY EXPRESS
# OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
# BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
# OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
# EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

%prefix "calc"

%source {
#include <stdio.h>
#include <stdlib.h>
}

statement <- _ e:expression _ EOL { printf("answer=%d\n", e); }
           / ( !EOL . )* EOL      { printf("error\n"); }

expression <- e:term { $$ = e; }

term <- l:term _ '+' _ r:factor { $$ = l + r; }
      / l:term _ '-' _ r:factor { $$ = l - r; }
      / e:factor                { $$ = e; }

factor <- l:factor _ '*' _ r:unary { $$ = l * r; }
        / l:factor _ '/' _ r:unary { $$ = l / r; }
        / e:unary                  { $$ = e; }

unary <- '+' _ e:unary { $$ = +e; }
       / '-' _ e:unary { $$ = -e; }
       / e:primary     { $$ = e; }

primary <- < [0-9]+ >               { $$ = atoi($1); }
         / '(' _ e:expression _ ')' { $$ = e; }

_      <- [ \t]*
EOL    <- '\n' / '\r\n' / '\r' / ';'

%%
int main() {
    calc_context_t *ctx = calc_create(NULL);
    while (calc_parse(ctx, NULL));
    calc_destroy(ctx);
    return 0;
}
)";
    printf("Check input: %d\n", Checker().validate_string(peg));
    printf("Check input: %d\n", Checker().validate_file("/home/h/prog/pegof/packcc/examples/calc-broken.peg"));
    return 0;

    Grammar g(peg);
    if (!g) {
        printf("ERROR: Failed to parse grammar!\n");
        exit(1);
    }
    //~ printf("%s\n", g.dump().c_str());
    //~ printf("------------------------\n");
    printf("%s\n", g.to_string().c_str());
    //~ printf("Check output: %d\n", Checker::validate(g.to_string()));
    //~ printf("------------------------\n");
    //~ printf("%s\n", g.type);
    //~ printf("%s\n", g.rules[0].type);
    //~ printf("%s\n", g.rules[0].expression.type);
    //~ for (int i = 0; i < g.size(); i++) {
        //~ printf("%d: %s\n", i, g[i]->type);
    //~ }

    //~ std::vector<Rule*> rules = g.find_all<Rule>([](const Rule& r){ return r.name == "EOL"; });
    //~ for (Rule* rule : rules) {
        //~ printf("- %s\n", rule->to_string().c_str());
    //~ }

    return 0;
}
