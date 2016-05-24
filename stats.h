class stats
{
  public:
    void reset();
    void process(double data);

    double average();
    double maxvalue();
    double variance();

  private:
    double sum;
    double sum2;
    double maxval;
    long counter;
};
